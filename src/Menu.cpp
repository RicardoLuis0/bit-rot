#include "Menu.h"
#include "Renderer.h"
#include "Log.h"
#include "Font.h"
#include "Config.h"
#include "Game.h"
#include "SaveData.h"
#include <string>

#include "SDL2Util.h"

constexpr bool canSave = false;

constexpr FakeString<80> BorderTop =    fixString(U"┌──────────────────────────────────────────────────────────────────────────────┐");
constexpr FakeString<80> BorderMid =    fixString(U"│                                                                              │");
constexpr FakeString<80> BorderBottom = fixString(U"└──────────────────────────────────────────────────────────────────────────────┘");

constexpr FakeString<42> Title[] {
    fixString(U"┌─────┐               ┌─────┐             "),
    fixString(U"│ ┌─┐ │  ┌─┐┌─────┐   │ ┌─┐ │┌─────┬─────┐"),
    fixString(U"│ └─┘ └┐ │ │└─┐ ┌─┘   │ └─┘ └┤ ┌─┐ ├─┐ ┌─┘"),
    fixString(U"│ ┌──┐ │ │ │  │ │     │ ┌──┐ │ │ │ │ │ │  "),
    fixString(U"│ └──┘ │ │ │  │ │     │ │  │ │ └─┘ │ │ │  "),
    fixString(U"└──────┘ └─┘  └─┘     └─┘  └─┴─────┘ └─┘  "),
};

constexpr FakeString<47> TitleSettings[] {
    fixString(U"┌──────┐                                       "),
    fixString(U"│ ┌────┼────┬─────┬─────┐┌─┐┌─────┬─────┬─────┐"),
    fixString(U"│ └────┤  ─┬┴─┐ ┌─┴─┐ ┌─┘│ ││ ┌─┐ │ ┌───┤ ┌───┘"),
    fixString(U"└────┐ │ ┌─┘  │ │   │ │  │ ││ │ │ │ │┌──┤ └───┐"),
    fixString(U"┌────┘ │ └──┐ │ │   │ │  │ ││ │ │ │ └┴─ ├───  │"),
    fixString(U"└──────┴────┘ └─┘   └─┘  └─┘└─┘ └─┴─────┴─────┘"),
};

constexpr FakeString SettingsTop =      fixString(U"┌──────────────┬───────────────────────────────────────────────────────────┐");
constexpr FakeString SettingsMid =      fixString(U"│              │                                                           │");
constexpr FakeString SettingsSep =      fixString(U"├──────────────┼───────────────────────────────────────────────────────────┤");
constexpr FakeString SettingsBottom =   fixString(U"└──────────────┴───────────────────────────────────────────────────────────┘");


constexpr FakeString<12> ButtonContinue[] {
    fixString(U"╔══════════╗"),
    fixString(U"║ Continue ║"),
    fixString(U"╚══════════╝"),
};

constexpr FakeString<16> ButtonContinueMaybe[] {
    fixString(U"╔══════════════╗"),
    fixString(U"║ Continue...? ║"),
    fixString(U"╚══════════════╝"),
};


constexpr FakeString<12> ButtonNew[] {
    fixString(U"╔══════════╗"),
    fixString(U"║ New Game ║"),
    fixString(U"╚══════════╝"),
};


constexpr FakeString<12> ButtonSettings[] {
    fixString(U"╔══════════╗"),
    fixString(U"║ Settings ║"),
    fixString(U"╚══════════╝"),
};

constexpr FakeString<12> ButtonQuit[] {
    fixString(U"╔══════════╗"),
    fixString(U"║   Quit   ║"),
    fixString(U"╚══════════╝"),
};

constexpr FakeString<17> ButtonSaveAndQuit[] {
    fixString(U"╔═══════════════╗"),
    fixString(U"║ Save And Quit ║"),
    fixString(U"╚═══════════════╝"),
};

int numMainMenuItems = 3;
int currentMainMenuItem = 0;

extern int currentScreen;
extern bool RunGame;

extern bool InGame;


void Menu::Init()
{
    currentMainMenuItem = SaveData::HasSave() ? -1 : 0;
}

static void DrawBorder()
{
    Renderer::DrawLineText(0, 0, BorderTop);
    for(int i = 1; i < 39; i++)
    {
        Renderer::DrawLineText(0, i, BorderMid);
    }
    Renderer::DrawLineText(0, 39, BorderBottom);
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
                SaveData::Reset();
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
    
    DrawBorder();
    
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

#define DRAW_BUTTON(var, i, name){\
    if(var == (i))\
    {\
        Renderer::DrawLineText(offsetX - 2, offsetY + 1, ">");\
        Renderer::DrawTextFillProp(offsetX, offsetY, name, CHAR_INVERT1 | CHAR_BLINK_INVERT | CHAR_BLINK3);\
    }\
    else\
    {\
        Renderer::DrawText(offsetX, offsetY, name);\
    }\
    offsetY += 4;}

void Menu::DrawMainMenu()
{
    bool hasSave = SaveData::HasSave();
    
    Renderer::DrawClear();
    
    DrawBorder();
    
    //Renderer::DrawText(1, 1, Title);
    Renderer::DrawText(19, 1, Title);
    
    int offsetX = 34;
    int offsetY = 8;
    
    if(hasSave) DRAW_BUTTON(currentMainMenuItem, -1, ButtonContinue);
    
    if(!hasSave && Config::getStringOr("SawIntro1", "no") == "yes")
    {
        offsetX = 32;
        DRAW_BUTTON(currentMainMenuItem, 0, ButtonContinueMaybe);
        offsetX = 34;
    }
    else
    {
        DRAW_BUTTON(currentMainMenuItem, 0, ButtonNew);
    }
    DRAW_BUTTON(currentMainMenuItem, 1, ButtonSettings);
    DRAW_BUTTON(currentMainMenuItem, 2, ButtonQuit);
}

void Menu::DrawPauseMenu()
{
    Renderer::DrawClear();
    
    DrawBorder();
    
    //Renderer::DrawText(1, 1, Title);
    Renderer::DrawText(19, 1, Title);
    
    int offsetX = 34;
    int offsetY = 8;
    
    DRAW_BUTTON(currentPauseMenuItem, 0, ButtonContinue);
    DRAW_BUTTON(currentPauseMenuItem, 1, ButtonSettings);
    
    offsetX = 32;
    
    DRAW_BUTTON(currentPauseMenuItem, 2, ButtonSaveAndQuit);
}
