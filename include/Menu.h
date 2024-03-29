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
    
    //border array must be 3x3
    void DrawBorder(const char **border, char prop = 0, int x = 0, int y = 0, int width = 80, int height = 40);
    void DrawBorderSingle(char prop = 0, int x = 0, int y = 0, int width = 80, int height = 40);
    void DrawBorderDouble(char prop = 0, int x = 0, int y = 0, int width = 80, int height = 40);
    
    void DrawLine(int x, int y, int width, char start, char mid, char end, char prop = 0);
}
