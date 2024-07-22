#pragma once

#include "Renderer.h"
#include "Common.h"
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
    
    template<Util::ContainerConvertibleTo<std::string_view> L>
    int DrawTextBox(int y, L messageLines, std::string_view message_end, bool end_inside)
    {
        Menu::DrawBorderSingle(0, 4, 10, 72, messageLines.size() + (end_inside ? 6 : 4));
        
        uint32_t offsetY = 12 + y;
        
        Util::ForEach(messageLines, [&offsetY](std::string_view v){
            Renderer::DrawLineTextCentered(offsetY++, v);
        });
        
        offsetY += end_inside ? 1 : 8;
        
        Renderer::DrawLineTextCentered(offsetY, message_end, CHAR_INVERT1 | CHAR_BLINK_INVERT | CHAR_BLINK3);
        
        return offsetY + (end_inside ? 2 : 0);
    }
    
    int DrawTextBox(int y, std::string_view message, std::string_view message_end, bool end_inside);
    
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
