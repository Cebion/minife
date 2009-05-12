/**
 * FreeDink mini-frontend

 * Copyright (C) 2009  Sylvain Beucler

 * This file is part of GNU FreeDink

 * GNU FreeDink is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 3 of the
 * License, or (at your option) any later version.

 * GNU FreeDink is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <math.h>

#include <iostream>
#include <string>
#include <sstream>

#include <guichan.hpp>
#include <guichan/sdl.hpp>
#include "SDL.h"
#include "SDL_image.h"
#include "SDL_framerate.h"

#include <unistd.h>

using namespace std;

/* Statically allocated string arrays, to store freedink.prx
   parameters. Don't malloc it since we're freeing all memory before
   starting FreeDink. */
char freedink_argv[5*1024][1024];

#ifdef _PSP
/* The following is necessary if you're running in PRX format, whose
   default heap size is only 64kB... */
#include <pspmoduleinfo.h>
PSP_HEAP_SIZE_MAX();

#include <pspsdk.h>
#include <pspsysmem.h>
#include <pspkernel.h>
extern "C" int _EXFUN(__psp_free_heap, (void));
#endif

enum buttons_psp {
  BUTTON_TRIANGLE=0, BUTTON_CIRCLE, BUTTON_CROSS, BUTTON_SQUARE,
  BUTTON_LTRIGGER, BUTTON_RTRIGGER,
  BUTTON_DOWN, BUTTON_LEFT, BUTTON_UP, BUTTON_RIGHT,
  BUTTON_SELECT, BUTTON_START, BUTTON_HOME, BUTTON_HOLD };

SDL_Surface* screen = NULL;
SDL_Surface* background = NULL;
SDL_Joystick *joy = NULL;
FPSmanager framerate_manager;

gcn::SDLImageLoader imageLoader;
gcn::ImageFont* font;


class Menu : public gcn::Gui
{
private:
  class DmodListModel : public gcn::ListModel
  {
  public:
    int getNumberOfElements()
    {
      return 100;
    }
    
    std::string getElementAt(int i)
    {
      std::ostringstream stm;
      stm << i;
      return stm.str();
    }
  };
  
  class TestActionListener : public gcn::ActionListener
  {
  public:
    void action(const gcn::ActionEvent& actionEvent)
    {
      cout << dynamic_cast<gcn::ListBox*>(actionEvent.getSource())->getSelected() << endl;
    }
  };

  gcn::SDLInput input;
  gcn::SDLGraphics graphics;

  gcn::Container top;

  gcn::Label label;
  DmodListModel lm;
  gcn::ListBox lb;
  gcn::ScrollArea scroll;
  TestActionListener al;
  gcn::CheckBox cb_sound;
  gcn::CheckBox cb_debug;
  gcn::CheckBox cb_m107;
  gcn::CheckBox cb_truecolor;


public:
  Menu(SDL_Surface* screen)
    : label("Hello World"), lb(&lm), scroll(&lb),
      cb_sound("Sound", true), cb_truecolor("True color", false),
      cb_m107("v1.07", false), cb_debug("Debug", false)
  {
    graphics.setTarget(screen);
  
    this->setGraphics(&graphics);
    this->setInput(&input);
    this->setTop(&top);

    top.setDimension(gcn::Rectangle(0, 50, 480, 222));
    top.setOpaque(false);

    top.add(&label, 400, 60);
    top.add(&scroll, 10, 10);
    top.add(&cb_sound,     270, 10);
    top.add(&cb_debug,     350, 10);
    top.add(&cb_m107,      270, 30);
    top.add(&cb_truecolor, 350, 30);

    scroll.setSize(250, 202);
    scroll.setOpaque(false);
    lb.setWidth(250-scroll.getScrollbarWidth());

    // The following is ludicrously slow on PSP, let's disable it
    //gcn::Color transparent = gcn::Color(0,0,0, 0);
    //lb.setBackgroundColor(transparent);

    lb.setSelected(0);
    lb.requestFocus();
    lb.addActionListener(&al);
  }

  ~Menu()
  {
  }

  gcn::Label& getLabel()
  {
    return label;
  }
};


Menu* gui;


void background_draw()
{
  static Uint32 i = 0;
  static Uint32 last_ticks = 0;
  static Uint32 acc = 0;
  if (last_ticks == 0)
    i = 10;
  else
    i = SDL_GetTicks() - last_ticks;
  acc += i;
  //if (acc > 1000)
  //  acc = 1000;
  int y = 0;
  for (; y < background->h; y += 2)
    {
      SDL_Rect src = {0, y, background->w, 2};
      SDL_Rect dst = {0, y};
      // I want 5 vertical sine waves on the 272-high screen, 1 wave
      // move per second (20 frames/s), 3 pixels horizontal size, with
      // PI=3.14
      dst.x = sin((y/272.0*2*3.14*5) + (acc*3.14/1000)) * 3;
      //dst.x = sin((y/272.0*2*3.14) * 50 * (1000-acc)/1000.0) * 10;
      SDL_BlitSurface(background, &src, screen, &dst);
    }
  last_ticks = SDL_GetTicks();
}


void init() 
{
  SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK);
  screen = SDL_SetVideoMode(480, 272, 32, SDL_HWSURFACE | SDL_DOUBLEBUF);
  SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 0,0,0));
  SDL_EnableUNICODE(1);
  SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);

  if (SDL_NumJoysticks() > 0) {
    SDL_JoystickOpen(0);
    SDL_JoystickEventState(SDL_ENABLE);
  }

  // Needed before using gcn::ImageFont
  gcn::Image::setImageLoader(&imageLoader);
  try
    {
      // Be sure to init it first, it sets the default size for all
      // widgets
      font = new gcn::ImageFont("rpgfont.png",
				" "
				"abcdefghijklmnopqrstuvwxyz"
				"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
				"0123456789"
				".,!?-+/():;%&`'*#=[]\"");
    }
  catch (gcn::Exception e)
    {
      cout << "Could not load font: " << e.getMessage() << endl;
      exit(1);
    }
  gcn::Widget::setGlobalFont(font);

  // Now we can create the widgets
  gui = new Menu(screen);

  SDL_Surface* img = IMG_Load("background.png");
  background = SDL_DisplayFormat(img);
  if (background == NULL)
    {
      cerr << "Error loading background: " << SDL_GetError() << endl;
      exit(1);
    }
  SDL_FreeSurface(img);

  SDL_initFramerate(&framerate_manager);
  SDL_setFramerate(&framerate_manager, 15);
}

void print_event_type(Uint8 type)
{
  cout << "Type = ";
  switch(type)
    {
    case SDL_ACTIVEEVENT:
      cout << "SDL_ActiveEvent";
      break;
    case SDL_KEYDOWN:
      cout << "SDL_KeyboardEvent/down";
      break;
    case SDL_KEYUP:
      cout << "SDL_KeyboardEvent/up";
      break;
    case SDL_MOUSEMOTION:
      cout << "SDL_MouseMotionEvent";
      break;
    case SDL_MOUSEBUTTONDOWN:
      cout << "SDL_MouseButtonEvent/down";
      break;
    case SDL_MOUSEBUTTONUP:
      cout << "SDL_MouseButtonEvent/up";
      break;
    case SDL_JOYAXISMOTION:
      cout << "SDL_JoyAxisEvent";
      break;
    case SDL_JOYBALLMOTION:
      cout << "SDL_JoyBallEvent";
      break;
    case SDL_JOYHATMOTION:
      cout << "SDL_JoyHatEvent";
      break;
    case SDL_JOYBUTTONDOWN:
      cout << "SDL_JoyButtonEvent/down";
      break;
    case SDL_JOYBUTTONUP:
      cout << "SDL_JoyButtonEvent/up";
      break;
    case SDL_QUIT:
      cout << "SDL_QuitEvent";
      break;
    case SDL_SYSWMEVENT:
      cout << "SDL_SysWMEvent";
      break;
    case SDL_VIDEORESIZE:
      cout << "SDL_ResizeEvent";
      break;
    case SDL_VIDEOEXPOSE:
      cout << "SDL_ExposeEvent";
      break;
    case SDL_USEREVENT:
      cout << "SDL_UserEvent";
      break;
    }
  cout << endl;
}

void run()
{
  bool running = true;

  while (running)
    {
      SDL_Event event;
      while (SDL_PollEvent(&event))
        {
	  if (event.type == SDL_KEYDOWN)
            {
	      if (event.key.keysym.sym == SDLK_ESCAPE)
                {
		  running = false;
		}
            }
	  else if(event.type == SDL_JOYBUTTONDOWN || event.type == SDL_JOYBUTTONUP)
	    {
	      if (event.jbutton.button == BUTTON_CIRCLE)
		{
		  running = false;
		}
	      else
		{
		  /* Simulate keyboard events that Guichan can
		     understand and apply */
		  SDL_Event synth_ev;
		  synth_ev.key.keysym.mod = KMOD_NONE;
		  synth_ev.key.keysym.sym = SDLK_UNKNOWN;
		  if (event.type == SDL_JOYBUTTONDOWN)
		    {
		      synth_ev.type = SDL_KEYDOWN;
		      synth_ev.key.state = SDL_PRESSED;
		    }
		  else
		    {
		      synth_ev.type = SDL_KEYUP;
		      synth_ev.key.state = SDL_RELEASED;
		    }
		  if (event.jbutton.button == BUTTON_CROSS)
		    {
		      synth_ev.key.keysym.sym = SDLK_RETURN;
		    }
		  else if (event.jbutton.button == BUTTON_DOWN)
		    {
		      synth_ev.key.keysym.sym = SDLK_DOWN;
		    }
		  else if (event.jbutton.button == BUTTON_UP)
		    {
		      synth_ev.key.keysym.sym = SDLK_UP;
		    }
		  else if (event.jbutton.button == BUTTON_LEFT)
		    {
		      synth_ev.key.keysym.sym = SDLK_TAB;
		      synth_ev.key.keysym.mod = (SDLMod)(synth_ev.key.keysym.mod | KMOD_LSHIFT);
		    }
		  else if (event.jbutton.button == BUTTON_RIGHT)
		    {
		      synth_ev.key.keysym.sym = SDLK_TAB;
		    }
		  
		  if (synth_ev.key.keysym.sym != SDLK_UNKNOWN)
		    {
		      if (SDL_PushEvent(&synth_ev) < 0)
			cerr << "Cannot synthetize event: " << SDL_GetError() << endl;
		    }
		}

	    }
	  else if(event.type == SDL_QUIT)
            {
	      running = false;
            }
	  
	  dynamic_cast<gcn::SDLInput*>(gui->getInput())->pushInput(event);
	  gui->logic();
        }
      
      string s;
      stringstream ss;
      ss << SDL_GetTicks();
      ss >> s;
      gui->getLabel().setCaption(s);

      background_draw();
      gui->draw();
      
      SDL_Flip(screen);
      SDL_framerateDelay(&framerate_manager);
    }
}

void halt()
{
  if (SDL_JoystickOpened(0))
    SDL_JoystickClose(joy);

  free(background);

  delete gui;
  delete font;
  
  SDL_Quit();
}

extern "C" int main(int argc, char **argv)
{
  int initial_free_memory = sceKernelTotalFreeMemSize()/1024;
  int initial_max_free_block = sceKernelMaxFreeMemSize()/1024;

  /* Run the nifty frontend */
  init();
  run();  
  halt();

#ifdef _PSP
  printf("Checking memory before doing anything:\n");
  printf("Memory free: %dkB\n", initial_free_memory);
  printf("Max free block: %dkB\n", initial_max_free_block);
  printf("\n");

  /* Release all memory for FreeDink - no more malloc from now on! */
  __psp_free_heap();

  printf("Now I'm cleaning up (freeing memory):\n");
  printf("Memory free: %dkB\n", sceKernelTotalFreeMemSize()/1024);
  printf("Max free block: %dkB\n", sceKernelMaxFreeMemSize()/1024);
  printf("\n");

  /* Run Dink */
  char* myargv[] = { (char*)"-d" };
//   SceUID mod = pspSdkLoadStartModuleWithArgs("host0:/freedink/cross-psp/src/freedink.prx",
// 					     PSP_MEMORY_PARTITION_USER, 1, myargv);
  SceUID mod = pspSdkLoadStartModuleWithArgs("./microfe.prx",
					     PSP_MEMORY_PARTITION_USER, 1, myargv);
  if (mod < 0)
    {
      // Error
      pspDebugScreenInit();
      pspDebugScreenPrintf("Could not run microfe:\n");
      switch(mod)
	{
	case 0x80010002:
	  pspDebugScreenPrintf("Program not found");
	  break;
	case 0x80020148: // SCE_KERNEL_ERROR_UNSUPPORTED_PRX_TYPE
	  pspDebugScreenPrintf("Unsupported PRX application");
	  break;
	case 0x800200D9: // http://forums.ps2dev.org/viewtopic.php?t=11887
	  pspDebugScreenPrintf("Not enough memory\n");
	  break;
	case 0x80020149: // SCE_KERNEL_ERROR_ILLEGAL_PERM_CALL
	  pspDebugScreenPrintf("Not running from memory card?");
	  break;
	case 0x80010014:
	  pspDebugScreenPrintf("Invalid path?");
	  break;
	case 0x8002013c: // SCE_KERNEL_ERROR_LIBRARY_NOTFOUND
	  pspDebugScreenPrintf("This user module should be compiled in kernel mode");
	  break;
	default:
	  pspDebugScreenPrintf("Unknown error", mod);
	}
      pspDebugScreenPrintf(" (%p)\n", mod);
      pspDebugScreenPrintf("\n");
      pspDebugScreenPrintf("If you think that's a bug, please write to " PACKAGE_BUGREPORT);
      sleep(5);
      /* Pause the thread, so that the user still can use the [HOME]
	 button to reset to the XMB. */
      sceKernelSleepThread();

      // never reached
      sceKernelExitGame();
      return EXIT_FAILURE;
    }
  // Don't printf anything here, it messes the SDL video
  // initialization for some reason..

  // Completely free all memory
  sceKernelSelfStopUnloadModule(1, 0, NULL);
#endif

  return EXIT_SUCCESS;
}
