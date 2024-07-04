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

bool showSecondNewGamePopup = false;
int numMainMenuPopupItems = 3;
int currentMainMenuPopupItem = 2;

void Menu::MainMenuResponder(SDL_Event *e)
{
    if(e->type == SDL_KEYDOWN)
    {
        if(showSecondNewGamePopup)
        {
            if(e->key == SDLK_UP)
            {
                Renderer::ResetTimer();
                currentMainMenuPopupItem--;
                if(currentMainMenuPopupItem < 0) currentMainMenuPopupItem = (numMainMenuPopupItems - 1);
            }
            else if(e->key == SDLK_DOWN)
            {
                Renderer::ResetTimer();
                currentMainMenuPopupItem = (currentMainMenuPopupItem + 1) % numMainMenuPopupItems;
            }
            else if(e->key == SDLK_RETURN)
            {
                switch(currentMainMenuPopupItem)
                {
                case 0: // first intro
                    Config::setString("SawIntro1", "no");
                    Game::ToIntro();
                    break;
                case 1: // new game
                    Config::setString("SawIntro1", "yes");
                    Game::ToIntro();
                    break;
                case 2: // quit
                    showSecondNewGamePopup = false;
                    break;
                }
            }
            else if(e->key == SDLK_ESCAPE)
            {
                showSecondNewGamePopup = false;
            }
            else if(e->key == SDLK_r)
            {
                Renderer::Compile();
            }
        }
        else
        {
            if(e->key == SDLK_UP)
            {
                Renderer::ResetTimer();
                currentMainMenuItem--;
                if(currentMainMenuItem < (SaveData::HasSave() ? -1 : 0)) currentMainMenuItem = (numMainMenuItems - 1);
            }
            else if(e->key == SDLK_DOWN)
            {
                Renderer::ResetTimer();
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
                    if(SaveData::HasSave())
                    {
                        Renderer::ResetTimer();
                        showSecondNewGamePopup = true;
                        currentMainMenuPopupItem = 2;
                    }
                    else
                    {
                        Game::ToIntro();
                    }
                    break;
                case 1: // settings
                    currentScreen = 1;
                    break;
                case 2: // quit
                    RunGame = false;
                    break;
                }
            }
            else if(e->key == SDLK_r)
            {
                Renderer::Compile();
            }
        }
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
    
    if(showSecondNewGamePopup)
    {
        Renderer::DrawClear(10, 11, 60, 20);
        Menu::DrawBorderSingle(0, 10, 11, 60, 20);
        Renderer::DrawLineTextCentered(13, "Start a New Game? Current Progress will be Erased.");
        
        int y = 18;
        
        DrawMenuItemButton(currentMainMenuPopupItem == 0, y, "New Game");
        DrawMenuItemButton(currentMainMenuPopupItem == 1, y, "Skip First Intro");
        DrawMenuItemButton(currentMainMenuPopupItem == 2, y, "Cancel");
    }
}

