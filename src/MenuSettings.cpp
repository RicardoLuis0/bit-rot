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

extern int currentScreen;
extern bool InGame;

int numSettingsMenuItems = 4;
int currentSettingsMenuItem = 0;

struct SettingItem
{
    virtual ~SettingItem() = default;
    virtual std::string_view getName() const = 0;
    virtual std::string getValue() const = 0;
    virtual void ToggleUp() = 0;
    virtual void ToggleDown() = 0;
};

extern std::vector<SettingItem*> settings;

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
            settings[currentSettingsMenuItem]->ToggleUp();
        }
        else if(e->key == SDLK_LEFT)
        {
            settings[currentSettingsMenuItem]->ToggleDown();
        }
        break;
    }
}

void DrawSetting(bool selected, std::string_view name, std::string_view value, int &y, bool start, bool end, int width1 = 16, int width2 = 61)
{
    int fullWidth = (width1 + width2) - 1;
    int x = ((80 - fullWidth) / 2);
    
    if(start)
    {
        Menu::DrawHalfLine(x, y - 1, width1 - 1, BorderTop[0], BorderTop[1], 0);
        Menu::DrawLine(x + (width1 - 1), y - 1, width2, BorderTopEnd[0], BorderTopEnd[1], BorderTopEnd[2], 0);
    }
    else
    {
        Menu::DrawHalfLine(x, y - 1, width1 - 1, BorderSep[0], BorderSep[1], 0);
        Menu::DrawLine(x + (width1 - 1), y - 1, width2, BorderSepEnd[0], BorderSepEnd[1], BorderSepEnd[2], 0);
    }
    
    Menu::DrawHalfLine(x, y, width1 - 1, BorderMid[0], BorderMid[1], 0);
    Menu::DrawLine(x + (width1 - 1), y, width2, BorderMid[0], BorderMid[1], BorderMid[2], 0);
    
    int name_x = x + 4;
    int value_x = x + width1 + 2;
    
    if(selected)
    {
        Renderer::DrawLineText(4, y, ">");
        Renderer::DrawLineTextFillProp(name_x, y, name, CHAR_INVERT1 | CHAR_BLINK_INVERT | CHAR_BLINK3);
        Renderer::DrawLineTextFillProp(value_x, y, value, CHAR_INVERT1);
    }
    else
    {
        Renderer::DrawLineText(name_x, y, name);
        Renderer::DrawLineText(value_x, y, value);
    }
    
    if(end)
    {
        Menu::DrawHalfLine(x, y + 1, width1 - 1, BorderBottom[0], BorderBottom[1], 0);
        Menu::DrawLine(x + (width1 - 1), y + 1, width2, BorderBottomEnd[0], BorderBottomEnd[1], BorderBottomEnd[2], 0);
    }
    
    y += 2;
}

void DrawSettings(unsigned selection, const std::vector<SettingItem*> &settings, int width1 = 16, int width2 = 61)
{
    unsigned last = settings.size() - 1;
    int y = 9;
    for(unsigned i = 0; i < settings.size(); i++)
    {
        DrawSetting(i == selection, settings[i]->getName(), settings[i]->getValue(), y, i == 0, i == last);
    }
}

struct : SettingItem
{
    virtual std::string_view getName() const override { return "Font"; }
    virtual std::string getValue() const override { return std::string(Font::curFontName()); }
    virtual void ToggleUp() override { Renderer::CycleFont(); }
    virtual void ToggleDown() override { Renderer::CycleFontDown(); }
} FontSetting;

struct : SettingItem
{
    virtual std::string_view getName() const override { return "Color"; }
    virtual std::string getValue() const override { return std::string(Renderer::GetTextColorName()); }
    virtual void ToggleUp() override { Renderer::CycleTextColor(); }
    virtual void ToggleDown() override { Renderer::CycleTextColorDown(); }
} ColorSetting;

struct : SettingItem
{
    virtual std::string_view getName() const override { return "VSync"; }
    virtual std::string getValue() const override { return std::string(Config::mustGetString("VSync")); }
    virtual void ToggleUp() override { Renderer::CycleVSync(); }
    virtual void ToggleDown() override { Renderer::CycleVSyncDown(); }
} VSyncSetting;

struct : SettingItem
{
    virtual std::string_view getName() const override { return "Volume"; }
    virtual std::string getValue() const override { return std::to_string(Mix_MasterVolume(-1)); }
    
    virtual void ToggleUp() override
    {
        int vol = Config::setInt("Volume", std::clamp(Mix_MasterVolume(-1) + 10, 0, 100));
        Mix_MasterVolume(vol);
        Mix_VolumeMusic(vol);
    }
    
    virtual void ToggleDown() override
    {
        int vol = Config::setInt("Volume", std::clamp(Mix_MasterVolume(-1) - 10, 0, 100));
        Mix_MasterVolume(vol);
        Mix_VolumeMusic(vol);
    }
} VolumeSetting;

std::vector<SettingItem*> settings
{
    &FontSetting,
    &ColorSetting,
    &VSyncSetting,
    &VolumeSetting,
};

void Menu::DrawSettingsMenu()
{
    Renderer::DrawClear();
    
    DrawBorderSingle();
    
    Renderer::DrawText(16, 1, TitleSettings);
    
    Renderer::DrawLineText(2, 8, SettingsTop);
    
    DrawSettings(currentSettingsMenuItem, settings);
}
