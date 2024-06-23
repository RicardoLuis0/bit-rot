#pragma once

#include <string>

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
    
    void DrawHalfLine(int x, int y, int width, char start, char mid, char prop = 0);
    
    int DrawButton(int x, int y, int min_width, std::string_view text, bool highlight); //, bool center_x = true, bool center_y = true);
    
    void DrawMenuItemButton(bool selected, int &y, std::string_view text);
    /*
    template<Util::ContainerConvertibleTo<std::string_view> L>
    void DrawMenuItemButtonRow(int selected, int &y, L texts)
    {
        int width = 0;
        std::vector<int> sizes;
        for(std::string_view text : texts)
        {
            int size = std::max<int>(min_width, text.size() + 4);
            sizes.push_back(size);
            width += size + 1;
        }
        width -= 1;
    }
    */
}
