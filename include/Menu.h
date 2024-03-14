#pragma once

union SDL_Event;

namespace Menu
{
    void MainMenuResponder(SDL_Event *e);
    void SettingsMenuResponder(SDL_Event *e);
    void PauseMenuResponder(SDL_Event *e);
    void DrawMainMenu();
    void DrawSettingsMenu();
    void DrawPauseMenu();
}
