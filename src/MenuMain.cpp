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

int numMainMenuItems = 3;
int currentMainMenuItem = 0;

void Menu::MainMenuResponder(SDL_Event *e)
{
    switch(e->type)
    {
    case SDL_KEYDOWN:
        if(e->key == SDLK_UP)
        {
            currentMainMenuItem--;
            if(currentMainMenuItem < (SaveData::HasSave() ? -1 : 0)) currentMainMenuItem = (numMainMenuItems - 1);
        }
        else if(e->key == SDLK_DOWN)
        {
            currentMainMenuItem++;
            
            if(currentMainMenuItem >= numMainMenuItems) currentMainMenuItem = SaveData::HasSave() ? -1 : 0;
        }
        else if(e->key == SDLK_RETURN)
        {
            switch(currentMainMenuItem)
            {
            case -1: // continue
                Game::DoLoad();
                Game::ToIntro();
                break;
            case 0: // new game
                Game::ToIntro();
                break;
            case 1: // settings
                currentScreen = 1;
                break;
            case 2: // quit
                RunGame = false;
                break;
            }
        }
        break;
    }
}

void Menu::DrawMainMenu()
{
    bool hasSave = SaveData::HasSave();
    
    Renderer::DrawClear();
    
    DrawBorderSingle();
    
    //Renderer::DrawText(1, 1, Title);
    Renderer::DrawText(19, 1, Title);
    
    int y = 9;
    
    if(hasSave) DrawMenuItemButton(currentMainMenuItem == -1, y, "Continue");
    
    DrawMenuItemButton(currentMainMenuItem == 0, y, (!hasSave && Config::getStringOr("SawIntro1", "no") == "yes") ? "Continue...?" : "New Game");
    
    DrawMenuItemButton(currentMainMenuItem == 1, y, "Settings");
    DrawMenuItemButton(currentMainMenuItem == 2, y, "Quit");
}

