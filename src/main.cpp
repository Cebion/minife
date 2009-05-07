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

#ifdef _PSP
/* The following is necessary if you're running in PRX format, whose
   default heap size is only 64kB... */
#include <pspmoduleinfo.h>
PSP_HEAP_SIZE_MAX();

#include <pspsdk.h>
#include <pspsysmem.h>
extern "C" int _EXFUN(__psp_free_heap, (void));
#endif

enum buttons_psp {
  BUTTON_TRIANGLE=0, BUTTON_CIRCLE, BUTTON_CROSS, BUTTON_SQUARE,
  BUTTON_LTRIGGER, BUTTON_RTRIGGER,
  BUTTON_DOWN, BUTTON_LEFT, BUTTON_UP, BUTTON_RIGHT,
  BUTTON_SELECT, BUTTON_START, BUTTON_HOME, BUTTON_HOLD };


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


SDL_Surface* screen = NULL;
SDL_Surface* background = NULL;
SDL_Joystick *joy = NULL;
FPSmanager framerate_manager;

gcn::SDLInput* input;
gcn::SDLGraphics* graphics;
gcn::SDLImageLoader* imageLoader;

gcn::Gui* gui;
gcn::Container* top;
gcn::ImageFont* font;
gcn::Label* label;
DmodListModel* lm;
gcn::ListBox* lb;


void background_draw()
{
  static double i = 0;
  static Uint32 last_ticks = 0;
  double delta = (SDL_GetTicks()-last_ticks) / 1000.0;
  i += delta;
  // SDL_Rect fill_left = {0, 0, 2, background->h};
  // SDL_Rect fill_right = {background->w, 0, 2, background->h};
  // SDL_FillRect(screen, &fill_left, SDL_MapRGB(screen->format, 0, 0, 0));
  // SDL_FillRect(screen, &fill_right, SDL_MapRGB(screen->format, 0, 0, 0));
  SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 0, 0, 0));
  //SDL_BlitSurface(background, NULL, screen, NULL);
  for (int y = 0; y < background->h; y += 4)
    {
      SDL_Rect src = {0, y, background->w, 4};
      SDL_Rect dst = {0, y};
      // I want 10 vertical sine waves on the 272-high screen, 1 wave
      // move per second (20 frames/s), 3 pixels horizontal size, with
      // PI=3.14
      dst.x = sin((y/272.0*3.14*10) + (i*3.14)) * 3;
      SDL_BlitSurface(background, &src, screen, &dst);
    }
  last_ticks = SDL_GetTicks();
}


void init() 
{
  SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK);
  screen = SDL_SetVideoMode(480, 272, 32, SDL_HWSURFACE | SDL_DOUBLEBUF);
  SDL_EnableUNICODE(1);
  SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);

  if (SDL_NumJoysticks() > 0) {
    SDL_JoystickOpen(0);
    SDL_JoystickEventState(SDL_ENABLE);
  }

  imageLoader = new gcn::SDLImageLoader();
  gcn::Image::setImageLoader(imageLoader);
  graphics = new gcn::SDLGraphics();
  graphics->setTarget(screen);
  input = new gcn::SDLInput();

  try
    {
      font = new gcn::ImageFont("rpgfont.png", " abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789");
    }
  catch (gcn::Exception e)
    {
      cout << "Could not load font: " << e.getMessage() << endl;
      exit(1);
    }
  gcn::Widget::setGlobalFont(font); 
  
  gui = new gcn::Gui();
  gui->setGraphics(graphics);
  gui->setInput(input);

  top = new gcn::Container();    
  top->setDimension(gcn::Rectangle(0, 0, 480, 272));
  top->setOpaque(false);
  gui->setTop(top);

  label = new gcn::Label("Hello World");
  top->add(label, 200, 100);

  lm = new DmodListModel();
  lb = new gcn::ListBox(lm);
  gcn::Color transparent = gcn::Color(0,0,0, 0);
  lb->setBackgroundColor(transparent);
  lb->setTabOutEnabled(false);
  lb->setSelected(0);
  gcn::ScrollArea* scroll = new gcn::ScrollArea(lb);
  scroll->setOpaque(false);
  scroll->setSize(460, 252);
  top->add(scroll, 10, 10);
  lb->requestFocus();
  lb->addActionListener(new TestActionListener());


  SDL_initFramerate(&framerate_manager);
  SDL_setFramerate(&framerate_manager, 15);

  SDL_Surface* img = IMG_Load("background.png");
  background = SDL_DisplayFormat(img);
  if (background == NULL)
    cerr << "Error loading background: " << SDL_GetError() << endl;
  SDL_FreeSurface(img);
  SDL_SetAlpha(background, SDL_SRCALPHA, 128); // 50% transparency
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
	  print_event_type(event.type);
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
		    synth_ev.key.keysym.sym = SDLK_RETURN;
		  else if (event.jbutton.button == BUTTON_DOWN)
		    synth_ev.key.keysym.sym = SDLK_DOWN;
		  else if (event.jbutton.button == BUTTON_UP)
		    synth_ev.key.keysym.sym = SDLK_UP;
		  
		  if (synth_ev.key.keysym.sym != SDLK_UNKNOWN)
		    {
		      cout << synth_ev.key.keysym.sym << endl;
		      if (SDL_PushEvent(&synth_ev) < 0)
			cerr << "Cannot synthetize event: " << SDL_GetError() << endl;
		    }
		}

	    }
	  else if(event.type == SDL_QUIT)
            {
	      running = false;
            }
	  
	  input->pushInput(event);
	  gui->logic();
        }
      
      string s;
      stringstream ss;
      ss << SDL_GetTicks();
      ss >> s;
      label->setCaption(s);

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

  delete lm;
  delete lb;
  delete label;
  delete font;
  delete top;
  delete gui;
  
  delete input;  
  delete graphics;
  delete imageLoader;
  
  SDL_Quit();
}

extern "C" int main(int argc, char **argv)
{
  /* Run the nifty frontend */
  init();
  run();  
  halt();

#ifdef _PSP
  /* Release all memory for FreeDink - no more malloc from now on! */
  // pspDebugScreenPrintf("Memory free: %dkB\n", sceKernelTotalFreeMemSize()/1024);
  __psp_free_heap();

  /* Run Dink */
  char* myargv[] = { (char*)"-d" };
  //SceUID mod = pspSdkLoadStartModuleWithArgs("host0:/freedink/cross-psp/src/freedink.prx", PSP_MEMORY_PARTITION_USER, 1, myargv);
  SceUID mod = pspSdkLoadStartModuleWithArgs("host0:/freedink/cross-psp/src/freedink.prx", PSP_MEMORY_PARTITION_USER, 1, myargv);
  if (mod < 0)
    {
      // Error
      pspDebugScreenInit();
      pspDebugScreenPrintf("Could not run FreeDink:\n");
      switch(mod)
	{
	case 0x80010002:
	  pspDebugScreenPrintf("Program not found (%p)\n", mod);
	  break;
	case 0x80020148: // SCE_KERNEL_ERROR_UNSUPPORTED_PRX_TYPE
	  pspDebugScreenPrintf("Unsupported PRX application (%p)\n", mod);
	  break;
	case 0x800200D9: // http://forums.ps2dev.org/viewtopic.php?t=11887
	  pspDebugScreenPrintf("Not enough memory (%p)\n", mod);
	  break;
	case 0x80020149: // SCE_KERNEL_ERROR_ILLEGAL_PERM_CALL
	  pspDebugScreenPrintf("Not running from memory card? (%p)\n", mod);
	  break;
	case 0x80010014:
	  pspDebugScreenPrintf("Invalid path? (%p)\n", mod);
	  break;
	default:
	  pspDebugScreenPrintf("Unknown error %p\n", mod);
	}
      pspDebugScreenPrintf("\n");
      pspDebugScreenPrintf("If you think that's a bug, please write to " PACKAGE_BUGREPORT);
      sleep(5);
      return EXIT_FAILURE;
    }
  // Don't printf anything here, it messes the SDL video
  // initialization for some reason..


  /* Pause the thread, so that if running FreeDink failed, the user
     still can use the [HOME] button to reset to the XMB. */
  // sceKernelSelfStopUnloadModule(1, 0, NULL); // cleaner, but [HOME] crashes
  sceKernelSleepThread();
#endif

  return EXIT_SUCCESS;
}
