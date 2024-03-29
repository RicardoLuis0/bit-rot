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
extern bool InGame;

int numSettingsMenuItems = 4;
int currentSettingsMenuItem = 0;

void Menu::SettingsMenuResponder(SDL_Event *e)
{
    switch(e->type)
    {
    case SDL_KEYDOWN:
        if(e->key == SDLK_ESCAPE)
        {
            currentScreen = InGame ? 2 : 0; // 2 = pause menu, 0 = main menu
        }
        else if(e->key == SDLK_UP)
        {
            currentSettingsMenuItem--;
            if(currentSettingsMenuItem < 0) currentSettingsMenuItem = (numSettingsMenuItems - 1);
        }
        else if(e->key == SDLK_DOWN)
        {
            currentSettingsMenuItem = (currentSettingsMenuItem + 1) % numSettingsMenuItems;
        }
        else if(e->key == SDLK_RETURN || e->key == SDLK_RIGHT)
        {
            switch(currentSettingsMenuItem)
            {
            case 0:
                Renderer::CycleFont();
                break;
            case 1:
                Renderer::CycleTextColor();
                break;
            case 2:
                Renderer::CycleVSync();
                break;
            case 3:
                {
                    int vol = Config::setInt("Volume", std::clamp(Mix_MasterVolume(-1) + 10, 0, 100));
                    Mix_MasterVolume(vol);
                    Mix_VolumeMusic(vol);
                    break;
                }
            }
        }
        else if(e->key == SDLK_LEFT)
        {
            switch(currentSettingsMenuItem)
            {
            case 0:
                Renderer::CycleFontDown();
                break;
            case 1:
                Renderer::CycleTextColorDown();
                break;
            case 2:
                Renderer::CycleVSyncDown();
                break;
            case 3:
                {
                    int vol = Config::setInt("Volume", std::clamp(Mix_MasterVolume(-1) - 10, 0, 100));
                    Mix_MasterVolume(vol);
                    Mix_VolumeMusic(vol);
                    break;
                }
            }
        }
        break;
    }
}

#define DRAW_SETTING(i, name, value){\
    if(currentSettingsMenuItem == (i))\
    {\
        Renderer::DrawLineText(offsetX1 - 2, offsetY, ">");\
        Renderer::DrawLineTextFillProp(offsetX1, offsetY, name, CHAR_INVERT1 | CHAR_BLINK_INVERT | CHAR_BLINK3);\
        Renderer::DrawLineTextFillProp(offsetX2, offsetY, value, CHAR_INVERT1);\
    }\
    else\
    {\
        Renderer::DrawLineText(offsetX1, offsetY, name);\
        Renderer::DrawLineText(offsetX2, offsetY, value);\
    }\
    offsetY += 2;}

void Menu::DrawSettingsMenu()
{
    Renderer::DrawClear();
    
    DrawBorderSingle();
    
    Renderer::DrawText(16, 1, TitleSettings);
    
    Renderer::DrawLineText(2, 8, SettingsTop);
    int offset = 9;
    for(int i = 0; i < numSettingsMenuItems; i++)
    {
        if(i != 0) Renderer::DrawLineText(2, offset - 1, SettingsSep);
        Renderer::DrawLineText(2, offset, SettingsMid);
        offset += 2;
    }
    Renderer::DrawLineText(2, offset - 1, SettingsBottom);
    
    int offsetX1;
    int offsetX2 = 19;
    
    int offsetY = 9;
    
    
    
    offsetX1 = 7;
    DRAW_SETTING(0, "Font", Font::curFontName());
    
    offsetX1 = 7;
    DRAW_SETTING(1, "Color", Renderer::GetTextColorName());
    
    offsetX1 = 7;
    DRAW_SETTING(2, "VSync", Config::mustGetString("VSync"));
    
    offsetX1 = 6;
    DRAW_SETTING(3, "Volume", std::to_string(Mix_MasterVolume(-1)));
    
}
