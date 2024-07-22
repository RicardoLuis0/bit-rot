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

extern int currentMainMenuItem;

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

void Menu::DrawHalfLine(int x, int y, int width, char start, char mid, char prop)
{
    if(width > 2)
    {
        Renderer::DrawChar(x, y, start, prop);
        Renderer::DrawFillLineTextProp(x + 1, y, mid, prop, width - 1);
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

int Menu::DrawButton(int x_center, int y_center, int min_width, std::string_view text, bool highlight) //, bool center_x, bool center_y)
{
    int width = std::max<int>(min_width, text.size() + 4);
    int x = x_center - (width / 2);
    Menu::DrawBorderDouble(highlight ? CHAR_INVERT2 | CHAR_BLINK_INVERT | CHAR_BLINK3 : 0, x, y_center - 1, width, 3);
    int text_x = x_center - (text.size() / 2);
    Renderer::DrawLineText(text_x, y_center, text, text.size());
    return x;
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

int Menu::DrawTextBox(int y, std::string_view message, std::string_view message_end, bool end_inside)
{
    return DrawTextBox(y, Util::SplitLines(message, 50), message_end, end_inside);
}
