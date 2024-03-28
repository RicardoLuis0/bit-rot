#pragma once

union SDL_Event;

namespace Menu
{
    void Init();
    inline void Quit() {};
    
    void MainMenuResponder(SDL_Event *e);
    void SettingsMenuResponder(SDL_Event *e);
    void PauseMenuResponder(SDL_Event *e);
    void DrawMainMenu();
    void DrawSettingsMenu();
    void DrawPauseMenu();
}
