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

#include <stdlib.h>

#include <iostream>
#include <string>
#include <sstream>

#include <guichan.hpp>
#include <guichan/sdl.hpp>
#include "SDL.h"

using namespace std;

#ifdef _PSP
/* The following is necessary if you're running in PRX format, whose
   default heap size is only 64kB... */
#include <pspmoduleinfo.h>
PSP_HEAP_SIZE_MAX();
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
  label->setPosition(200, 100);
  top->add(label); 
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
  init();
  run();  
  halt();

  return EXIT_SUCCESS;
}
