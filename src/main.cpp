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
#include <string.h>

#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <algorithm>

#include <guichan.hpp>
#include <guichan/sdl.hpp>
#include "SDL.h"
#include "SDL_image.h"
#include "SDL_framerate.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <stdio.h>
#include <cstring>

using namespace std;

/* Statically allocated string arrays, to store freedink parameters. */
int freedink_argc = 0; // Initialize at 0 since we'll dynamically add arguments
char freedink_argv[8][1024]; // Support up to 8 arguments (including --game and dmod)

/* SDL Variables */
SDL_Surface* screen = NULL;
SDL_Surface* background = NULL;
FPSmanager framerate_manager;

gcn::SDLImageLoader imageLoader;
gcn::ImageFont* font;

class Menu : public gcn::Gui
{
private:
    class DmodListModel : public gcn::ListModel
    {
    private:
        vector<string> dmods;

    public:
        DmodListModel()
        {
            // Open the current directory and add subdirectories to the dmod list
            DIR *dir = opendir(".");
            if (dir)
            {
                struct dirent *entry;
                while ((entry = readdir(dir)) != NULL)
                {
                    // Skip "." and ".."
                    if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
                        continue;

                    // Check if it's a directory and add it to the list
                    struct stat buf;
                    stat(entry->d_name, &buf);
                    if (S_ISDIR(buf.st_mode))
                    {
                        dmods.emplace_back(entry->d_name);
                    }
                }
                closedir(dir);
                std::sort(dmods.begin(), dmods.end()); // Sort the list alphabetically
            }
        }

        int getNumberOfElements() override
        {
            return dmods.size();
        }

        std::string getElementAt(int i) override
        {
            return dmods.at(i);
        }
    };

    class TestActionListener : public gcn::ActionListener
    {
    public:
        void action(const gcn::ActionEvent& actionEvent) override
        {
            auto* lb = dynamic_cast<gcn::ListBox*>(actionEvent.getSource());
            if (lb)
            {
                cout << lb->getSelected() << ": " << lb->getListModel()->getElementAt(lb->getSelected()) << endl;
            }
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

public:
    gcn::CheckBox cb_sound{"Sound", true};
    gcn::CheckBox cb_debug{"Debug", false};
    gcn::CheckBox cb_m107{"v1.07", false};
    gcn::CheckBox cb_truecolor{"True color", false};

    Menu(SDL_Surface* screen)
        : label("Hello World"), lb(&lm), scroll(&lb)
    {
        // Set up graphics and input
        graphics.setTarget(screen);
        this->setGraphics(&graphics);
        this->setInput(&input);
        this->setTop(&top);

        // Configure layout and widgets
        top.setDimension(gcn::Rectangle(0, 50, 480, 222));
        top.setOpaque(true);

        top.add(&label, 400, 60);
        top.add(&scroll, 10, 10);
        top.add(&cb_sound, 270, 10);
        top.add(&cb_debug, 350, 10);
        top.add(&cb_m107, 270, 30);
        top.add(&cb_truecolor, 350, 30);

        scroll.setSize(250, 202);
        scroll.setOpaque(true);
        lb.setWidth(250 - scroll.getScrollbarWidth());

        lb.setSelected(0);
        lb.requestFocus();
        lb.addActionListener(&al);
    }

    ~Menu() override = default;

    gcn::Label& getLabel()
    {
        return label;
    }

    gcn::ListBox& getListBox()
    {
        return lb;
    }
};

// Global pointer to the Menu instance
Menu* gui;


void background_draw()
{
    static Uint32 last_ticks = 0;
    static Uint32 acc = 0;

    Uint32 current_ticks = SDL_GetTicks();
    Uint32 delta = (last_ticks == 0) ? 10 : current_ticks - last_ticks;
    acc += delta;

    const double PI = 3.141592653589793;
    const double screen_height = 272.0; // Adjust as needed

    for (int y = 0; y < background->h; ++y)
    {
        SDL_Rect src = {0, (Sint16)y, (Uint16)background->w, 1};
        SDL_Rect dst = {0, (Sint16)y};

        // Create 10 vertical sine waves on the screen, each moving 2 waves/second
        dst.x = static_cast<int>(sin((y / screen_height * 2 * PI * 10) + (acc * PI * 2 / 1000)) * 3);

        SDL_BlitSurface(background, &src, screen, &dst);
    }

    last_ticks = current_ticks;
}


void init() 
{
    // Initialize SDL with video support
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        cerr << "Unable to initialize SDL: " << SDL_GetError() << endl;
        exit(1);
    }

    // Set up the screen for fullscreen mode
    screen = SDL_SetVideoMode(480, 272, 32, SDL_HWSURFACE | SDL_DOUBLEBUF | SDL_FULLSCREEN);
    if (!screen)
    {
        cerr << "Unable to set video mode: " << SDL_GetError() << endl;
        exit(1);
    }

    // Fill the screen with black
    SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 0, 0, 0));

    // Enable Unicode and key repeat (for keyboard handling)
    SDL_EnableUNICODE(1);
    SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);

    // Set image loader for guichan and load font
    gcn::Image::setImageLoader(&imageLoader);
    try
    {
        font = new gcn::ImageFont("rpgfont.png",
                                  " "
                                  "abcdefghijklmnopqrstuvwxyz"
                                  "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                  "0123456789"
                                  ".,!?-+/():;%&`'*#=[]\"");
    }
    catch (gcn::Exception& e)
    {
        cerr << "Could not load font: " << e.getMessage() << endl;
        exit(1);
    }
    gcn::Widget::setGlobalFont(font);

    // Initialize the GUI
    gui = new Menu(screen);

    // Load and set the background image
    SDL_Surface* img = IMG_Load("background.png");
    if (!img)
    {
        cerr << "Error loading background image: " << SDL_GetError() << endl;
        exit(1);
    }
    background = SDL_DisplayFormat(img);
    SDL_FreeSurface(img);

    // Set up framerate management (15 FPS)
    SDL_initFramerate(&framerate_manager);
    SDL_setFramerate(&framerate_manager, 15);
}

void print_event_type(Uint8 type)
{
    switch (type)
    {
    case SDL_KEYDOWN:
        cout << "Event: SDL_KeyboardEvent (Key Down)" << endl;
        break;
    case SDL_KEYUP:
        cout << "Event: SDL_KeyboardEvent (Key Up)" << endl;
        break;
    case SDL_MOUSEBUTTONDOWN:
        cout << "Event: SDL_MouseButtonEvent (Button Down)" << endl;
        break;
    case SDL_MOUSEBUTTONUP:
        cout << "Event: SDL_MouseButtonEvent (Button Up)" << endl;
        break;
    case SDL_QUIT:
        cout << "Event: SDL_QuitEvent" << endl;
        break;
    default:
        cout << "Event: Other" << endl;
        break;
    }
}

void run()
{
    bool running = true;
    char current_path[1024];

    // Get the current working directory (where minife and freedink are located)
    if (getcwd(current_path, sizeof(current_path)) == NULL)
    {
        perror("getcwd() error");
        return;
    }

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
                else if (event.key.keysym.sym == SDLK_RETURN) // Trigger when Enter is pressed
                {
                    // Get the selected DMOD folder
                    string cur_dmod = gui->getListBox().getListModel()->getElementAt(gui->getListBox().getSelected());

                    if (!cur_dmod.empty())
                    {
                        // Reset the argument array and count
                        memset(freedink_argv, 0, sizeof(freedink_argv));
                        freedink_argc = 0;

                        // Add arguments based on the GUI selections
                        if (!gui->cb_sound.isSelected()) { strcpy(freedink_argv[freedink_argc++], "-s"); }
                        if (gui->cb_debug.isSelected()) { strcpy(freedink_argv[freedink_argc++], "-d"); }
                        if (gui->cb_m107.isSelected()) { strcpy(freedink_argv[freedink_argc++], "-7"); }
                        if (gui->cb_truecolor.isSelected()) { strcpy(freedink_argv[freedink_argc++], "-t"); }

                        // Add the --game argument and selected DMOD
                        strcpy(freedink_argv[freedink_argc++], "--game");
                        strcpy(freedink_argv[freedink_argc++], cur_dmod.c_str());

                        // Construct the freedink command
                        char freedink_cmd[2048];
                        snprintf(freedink_cmd, sizeof(freedink_cmd), "%s/freedink", current_path);

                        // Prepare arguments for execvp
                        char* argv[freedink_argc + 2]; // +2 for binary and NULL terminator
                        argv[0] = freedink_cmd;
                        for (int i = 0; i < freedink_argc; ++i) { argv[i + 1] = freedink_argv[i]; }
                        argv[freedink_argc + 1] = NULL;

                        // Execute freedink
                        execvp(freedink_cmd, argv);
                        perror("Error launching freedink"); // If execvp fails
                        exit(EXIT_FAILURE);
                    }
                }
            }
            else if (event.type == SDL_QUIT)
            {
                running = false;
            }

            // Process GUI input and logic
            dynamic_cast<gcn::SDLInput*>(gui->getInput())->pushInput(event);
            gui->logic();
        }

        // GUI updates
        gui->getLabel().setCaption(std::to_string(SDL_GetTicks()));
        background_draw();
        gui->draw();

        SDL_Flip(screen);  // Refresh the screen
        SDL_framerateDelay(&framerate_manager);  // Control frame rate
    }
}

void halt()
{
  SDL_FreeSurface(background);

  delete gui;
  delete font;
  
  SDL_Quit();
}

extern "C" int main()
{
    // Initialize the program
    init();

    // Run the main loop
    run();

    // Clean up and exit
    halt();

    return EXIT_SUCCESS;
}