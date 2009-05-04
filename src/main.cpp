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

#include <iostream>
#include <string>
#include <sstream>

#include <guichan.hpp>
#include <guichan/sdl.hpp>
#include "SDL.h"

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

SDL_Surface* screen;
SDL_Joystick *joy = NULL;
gcn::SDLInput* input;
gcn::SDLGraphics* graphics;
gcn::SDLImageLoader* imageLoader;

gcn::Gui* gui;
gcn::Container* top;
gcn::ImageFont* font;
gcn::Label* label;


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
    cout << actionEvent.getId() << endl;
    cout << dynamic_cast<gcn::ListBox*>(actionEvent.getSource())->getSelected() << endl;
  }
};


void init() 
{
  SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK);
  screen = SDL_SetVideoMode(480, 272, 0, SDL_HWSURFACE | SDL_DOUBLEBUF);
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
  gui->setTop(top);

  label = new gcn::Label("Hello World");
  top->add(label, 200, 100);

  DmodListModel* lm = new DmodListModel();
  gcn::ListBox* lb = new gcn::ListBox(lm);
  gcn::Color transparent = gcn::Color(0,0,0, 0);
  lb->setBackgroundColor(transparent);
  lb->setTabOutEnabled(false);
  lb->setSelected(0);
  top->add(lb, 125, 150);
  lb->requestFocus();
  lb->addActionListener(new TestActionListener());
}

void run()
{
  bool running = true;
  
  while (running)
    {
      SDL_Event event;
      while(SDL_PollEvent(&event))
        {
	  if (event.type == SDL_KEYDOWN)
            {
	      if (event.key.keysym.sym == SDLK_ESCAPE)
                {
		  running = false;
               }
            }
	  else if(event.type == SDL_JOYBUTTONDOWN)
	    {
	      if (event.jbutton.button == 1)
		running = false;
	    }
	  else if(event.type == SDL_QUIT)
            {
	      running = false;
            }
	  
	  input->pushInput(event);
        }
      
      string s;
      stringstream ss;
      ss << SDL_GetTicks();
      ss >> s;
      label->setCaption(s);

      gui->logic();
      gui->draw();
      
      SDL_Flip(screen);
    }
}

void halt()
{
  if (SDL_JoystickOpened(0))
    SDL_JoystickClose(joy);

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
