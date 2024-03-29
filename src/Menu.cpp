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

constexpr bool canSave = false;

int numMainMenuItems = 3;
int currentMainMenuItem = 0;

extern int currentScreen;
extern bool RunGame;

extern bool InGame;


void Menu::Init()
{
    currentMainMenuItem = SaveData::HasSave() ? -1 : 0;
}

void Menu::DrawLine(int x, int y, int width, char start, char mid, char end, char prop)
{
    if(width > 2)
    {
        Renderer::DrawChar(x, y, start, prop);
        Renderer::DrawFillLineTextProp(x + 1, y, mid, prop, width - 2);
        Renderer::DrawChar(x + (width - 1), y, end, prop);
    }
}

void Menu::DrawBorder(const char **border, char prop, int x, int y, int width, int height)
{
    if(width > 2 && height > 2)
    {
        DrawLine(x, y, width, border[0][0], border[0][1], border[0][2], prop);
        for(int i = 1; i < (height - 1); i++)
        {
            DrawLine(x, y + i, width, border[1][0], border[1][1], border[1][2], prop);
        }
        DrawLine(x, y + (height - 1), width, border[2][0], border[2][1], border[2][2], prop);
    }
}

void Menu::DrawBorderSingle(char prop, int x, int y, int width, int height)
{
    DrawBorder((std::array<const char*, 3> {BorderTop.data(), BorderMid.data(), BorderBottom.data()}).data(), prop, x, y, width, height);
}

void Menu::DrawBorderDouble(char prop, int x, int y, int width, int height)
{
    DrawBorder((std::array<const char*, 3> {DoubleBorderTop.data(), DoubleBorderMid.data(), DoubleBorderBottom.data()}).data(), prop, x, y, width, height);
}

int Menu::DrawButton(int x_center, int y_center, int min_width, std::string_view text, bool highlight)
{
    int width = std::max<int>(min_width, text.size() + 4);
    int x = x_center - (width / 2);
    Menu::DrawBorderDouble(highlight ? CHAR_INVERT2 | CHAR_BLINK_INVERT | CHAR_BLINK3 : 0, x, y_center - 1, width, 3);
    int text_x = x_center - (text.size() / 2);
    Renderer::DrawLineText(text_x, y_center, text, text.size());
    return x;
}

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

int numPauseMenuItems = 3;
int currentPauseMenuItem = 0;

void Menu::PauseMenuResponder(SDL_Event *e)
{
    switch(e->type)
    {
    case SDL_KEYDOWN:
        if(e->key == SDLK_UP)
        {
            currentPauseMenuItem--;
            if(currentPauseMenuItem < 0) currentPauseMenuItem = (numPauseMenuItems - 1);
        }
        else if(e->key == SDLK_DOWN)
        {
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
        break;
    }
}

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

void Menu::DrawMenuItemButton(bool selected, int &y, std::string_view text)
{
    if(selected)
    {
        Renderer::DrawLineText(DrawButton(40, y, 12, text, true) - 2, y, ">");
    }
    else
    {
        DrawButton(40, y, 12, text, false);
    }
    y += 4;
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
