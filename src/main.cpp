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
#include "SDL2Util.h"

#undef main

bool RunGame = true;

bool InGame = false;

int currentScreen = 0;

Input::Responder responders[]
{
    &Menu::MainMenuResponder,
    &Menu::SettingsMenuResponder,
    &Menu::PauseMenuResponder,
    &Game::IntroResponder,
    &Game::Responder,
    &Game::EndResponder,
};

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
                LogDebug("Starting Main Loop");
                
                #ifndef DEBUG_BUILD
                    if(Config::getStringOr("SawIntro1", "no") == "yes")
                    {
                        Audio::PlayMusic("tension");
                    }
                    else
                    {
                        Audio::PlayMusic("loop");
                    }
                #endif
                
                int vol = std::clamp<int>(Config::getIntOr("Volume", 100), 0, 100);
                Mix_MasterVolume(vol);
                Mix_VolumeMusic(vol);
                
                while(RunGame)
                {
                    if(!Input::Poll(responders[currentScreen])) break;
                    
                    switch(currentScreen)
                    {
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
        catch(std::exception &e)
        {
            LogError(e.what());
            Log::Disable();
            Util::ShowFatalError("Fatal Error", e.what());
            return EXIT_FAILURE;
        }
    }
    catch(std::exception &e)
    {
        Util::ShowFatalError("Fatal Error", e.what());
        return EXIT_FAILURE;
    }
    
    return EXIT_SUCCESS;
}

#ifdef DEBUG_BUILD
int main(int argc, char *argv[])
{
    return doGame();
}
#else

#define WIN32_LEAN_AND_MEAN
#include "windows.h"

int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR     lpCmdLine, int nShowCmd)
{
    return doGame();
}
#endif
