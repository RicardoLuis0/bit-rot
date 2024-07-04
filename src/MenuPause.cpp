#include "Menu.h"
#include "Renderer.h"
#include "Log.h"
#include "Font.h"
#include "Config.h"
#include "Game.h"
#include "SaveData.h"
#include "MenuStrings.h"
#include <string>

#include "SDL2Util.h"

extern int currentScreen;
extern bool RunGame;

int numPauseMenuItems = 3;
int currentPauseMenuItem = 0;

void Menu::PauseMenuResponder(SDL_Event *e)
{
    switch(e->type)
    {
    case SDL_KEYDOWN:
        if(e->key == SDLK_UP)
        {
            Renderer::ResetTimer();
            currentPauseMenuItem--;
            if(currentPauseMenuItem < 0) currentPauseMenuItem = (numPauseMenuItems - 1);
        }
        else if(e->key == SDLK_DOWN)
        {
            Renderer::ResetTimer();
            currentPauseMenuItem = (currentPauseMenuItem + 1) % numPauseMenuItems;
        }
        else if(e->key == SDLK_ESCAPE)
        {
            currentScreen = 4; // ingame
        }
        else if(e->key == SDLK_RETURN)
        {
            switch(currentPauseMenuItem)
            {
            case 0: // continue
                currentScreen = 4; // ingame
                break;
            case 1: // settings
                currentScreen = 1;
                break;
            case 2: // save and quit
                //Game::DoSave();
                RunGame = false;
                break;
            }
        }
        else if(e->key == SDLK_r)
        {
            Renderer::Compile();
        }
        break;
    }
}

void Menu::DrawPauseMenu()
{
    Renderer::DrawClear();
    
    DrawBorderSingle();
    
    Renderer::DrawText(19, 1, Title);
    
    int y = 9;
    
    DrawMenuItemButton(currentPauseMenuItem == 0, y, "Continue");
    DrawMenuItemButton(currentPauseMenuItem == 1, y, "Settings");
    
    DrawMenuItemButton(currentPauseMenuItem == 2, y, "Save And Quit");
}
