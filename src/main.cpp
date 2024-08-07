#include <iostream>
#include <cstdlib>

#include "Common.h"

#include "Log.h"
#include "Font.h"
#include "Input.h"
#include "System.h"
#include "Exceptions.h"
#include "Renderer.h"
#include "Config.h"
#include "Menu.h"
#include "Game.h"
#include "SaveData.h"
#include "SDL2Util.h"

#undef main

bool RunGame = true;

bool InGame = false;

int currentScreen = 0;


void NullResponder(SDL_Event *e)
{
}

std::map<uint32_t, Input::Responder> responders
{
    {0, &Menu::MainMenuResponder},
    {1, &Menu::SettingsMenuResponder},
    {2, &Menu::PauseMenuResponder},
    {3, &Game::IntroResponder},
    {4, &Game::Responder},
    {5, &Game::EndResponder},
    {999, &NullResponder},
};

int GetSoundVolume();
int GetMusicVolume();

int lastScreen = 0;

int doGame()
{
    try
    {
        UseSubsystem(System);
        UseSubsystem(Log);
        
        try
        {
            UseSubsystem(Config);
            UseSubsystem(Font);
            {
                UseSubsystem(Exceptions);
                UseSubsystem(Renderer);
                UseSubsystem(Input);
                UseSubsystem(Audio);
                UseSubsystem(SaveData);
                UseSubsystem(Menu);
                UseSubsystem(Game);
                
                LogDebug("Starting Main Loop");
                
                if(Config::getScriptStringOr("SawIntro1", "no") == "yes" || SaveData::HasSave())
                {
                    Audio::PlayMusic("tension");
                }
                else
                {
                    Audio::PlayMusic("loop");
                }
                
                Mix_MasterVolume(GetSoundVolume());
                Mix_VolumeMusic(GetMusicVolume());
                
                while(RunGame)
                {
                    if(!Input::Poll(responders[currentScreen])) break;
                    
                    switch(currentScreen)
                    {
                    case 999:
                    case 0:
                        InGame = false;
                        Menu::DrawMainMenu();
                        break;
                    case 1:
                        Menu::DrawSettingsMenu();
                        break;
                    case 2:
                        Menu::DrawPauseMenu();
                        break;
                    case 3:
                        InGame = true;
                        Game::TickIntro();
                        break;
                    case 4:
                        InGame = true;
                        Game::Tick();
                        break;
                    case 5:
                        Game::TickEnd();
                        break;
                    }
                    
                    Renderer::Render();
                }
            }
        }
        catch(TracedError &e)
        {
            LogError(e.what());
            Log::Disable();
            if(e.trace != "")
            {
                Util::ShowFatalError("Fatal Error", e.what() + std::string("\n\nstacktrace:\n") + e.trace);
            }
            else
            {
                Util::ShowFatalError("Fatal Error", e.what());
            }
            return EXIT_FAILURE;
        }
        catch(std::exception &e)
        {
            LogError(e.what());
            Log::Disable();
            Util::ShowFatalError("Fatal Error", e.what());
            return EXIT_FAILURE;
        }
    }
    catch(TracedError &e)
    {
        if(e.trace != "")
        {
            Util::ShowFatalError("Fatal Error", e.what() + std::string("\n\nstacktrace:\n") + e.trace);
        }
        else
        {
            Util::ShowFatalError("Fatal Error", e.what());
        }
        return EXIT_FAILURE;
    }
    catch(std::exception &e)
    {
        Util::ShowFatalError("Fatal Error", e.what());
        return EXIT_FAILURE;
    }
    
    return EXIT_SUCCESS;
}

#if defined(DEBUG_BUILD) || !defined(_WIN32)
int main(int argc, char *argv[])
{
    return doGame();
}
#else

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR     lpCmdLine, int nShowCmd)
{
    return doGame();
}
#endif
