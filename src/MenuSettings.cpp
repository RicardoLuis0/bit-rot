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

int currentSettingsMenuItem = 0;

constexpr int DefaultGlobalVolume = 50;
constexpr int DefaultBloomStrength = 100;
constexpr int DefaultCrtCurve = 100;
constexpr int DefaultSoundVolume = 100;
constexpr int DefaultMusicVolume = 100;

struct SettingItem
{
    virtual ~SettingItem() = default;
    virtual std::string_view getName() const = 0;
    virtual std::string getValue() const = 0;
    virtual void ToggleUp() = 0;
    virtual void ToggleDown() = 0;
};

int GetSoundVolume()
{
    double globalVol = Config::getIntOr("Volume", DefaultGlobalVolume) / 100.0;
    double soundVol = Config::getIntOr("VolumeSound", DefaultSoundVolume) / 100.0;
    double vol = globalVol * soundVol;
    
    return (vol * vol) * 64; // limit "max" volume to 50% of absolute max
}

int GetMusicVolume()
{
    if(Config::getIntOr("MuteMusic", 0)) return 0;
    
    double globalVol = Config::getIntOr("Volume", DefaultGlobalVolume) / 100.0;
    double musicVol = Config::getIntOr("VolumeMusic", DefaultMusicVolume) / 100.0;
    double vol = globalVol * musicVol;
    
    return (vol * vol) * 64; // limit "max" volume to 50% of absolute max
}

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
            Renderer::ResetTimer();
            currentSettingsMenuItem--;
            if(currentSettingsMenuItem < 0) currentSettingsMenuItem = (settings.size() - 1);
        }
        else if(e->key == SDLK_DOWN)
        {
            Renderer::ResetTimer();
            currentSettingsMenuItem = (currentSettingsMenuItem + 1) % settings.size();
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

void DrawSetting(bool selected, std::string_view name, std::string_view value, int &y, bool start, bool end, int width1 = 26, int width2 = 51)
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
        Renderer::MenuText.DrawLineText(4, y, ">");
        Renderer::MenuText.DrawLineTextFillProp(name_x, y, name, CHAR_INVERT1 | CHAR_BLINK_INVERT | CHAR_BLINK3);
        Renderer::MenuText.DrawLineTextFillProp(value_x, y, value, CHAR_INVERT1);
    }
    else
    {
        Renderer::MenuText.DrawLineText(name_x, y, name);
        Renderer::MenuText.DrawLineText(value_x, y, value);
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
    virtual std::string_view getName() const override { return "Compress Saves"; }
    virtual std::string getValue() const override { return std::string(Config::getStringOr("CompressSaves", "yes")); }
    virtual void ToggleUp() override
    {
        if(Config::getStringOr("CompressSaves", "yes") == "yes")
        {
            Config::setString("CompressSaves", "no");
        }
        else
        {
            Config::setString("CompressSaves", "yes");
        }
    }
    virtual void ToggleDown() override { ToggleUp(); }
} CompressSavesSetting;

struct : SettingItem
{
    virtual std::string_view getName() const override { return "Global Volume"; }
    virtual std::string getValue() const override { return std::to_string(Config::getIntOr("Volume", DefaultGlobalVolume)); }
    
    virtual void ToggleUp() override
    {
        Config::setInt("Volume", std::clamp<int>(Config::getIntOr("Volume", DefaultGlobalVolume) + 10, 0, 100));
        Mix_MasterVolume(GetSoundVolume());
        Mix_VolumeMusic(GetMusicVolume());
    }
    
    virtual void ToggleDown() override
    {
        Config::setInt("Volume", std::clamp<int>(Config::getIntOr("Volume", DefaultGlobalVolume) - 10, 0, 100));
        Mix_MasterVolume(GetSoundVolume());
        Mix_VolumeMusic(GetMusicVolume());
    }
} VolumeSetting;

struct : SettingItem
{
    virtual std::string_view getName() const override { return "Sound Volume"; }
    virtual std::string getValue() const override { return std::to_string(Config::getIntOr("VolumeSound", DefaultSoundVolume)); }
    
    virtual void ToggleUp() override
    {
        Config::setInt("VolumeSound", std::clamp<int>(Config::getIntOr("VolumeSound", DefaultSoundVolume) + 10, 0, 100));
        Mix_MasterVolume(GetSoundVolume());
    }
    
    virtual void ToggleDown() override
    {
        Config::setInt("VolumeSound", std::clamp<int>(Config::getIntOr("VolumeSound", DefaultSoundVolume) - 10, 0, 100));
        Mix_MasterVolume(GetSoundVolume());
    }
} VolumeSoundSetting;

struct : SettingItem
{
    virtual std::string_view getName() const override { return "Music Volume"; }
    virtual std::string getValue() const override { return std::to_string(Config::getIntOr("VolumeMusic", DefaultMusicVolume)); }
    
    virtual void ToggleUp() override
    {
        Config::setInt("VolumeMusic", std::clamp<int>(Config::getIntOr("VolumeMusic", DefaultMusicVolume) + 10, 0, 100));
        Mix_VolumeMusic(GetMusicVolume());
    }
    
    virtual void ToggleDown() override
    {
        Config::setInt("VolumeMusic", std::clamp<int>(Config::getIntOr("VolumeMusic", DefaultMusicVolume) - 10, 0, 100));
        Mix_VolumeMusic(GetMusicVolume());
    }
} VolumeMusicSetting;

struct : SettingItem
{
    virtual std::string_view getName() const override { return "Music"; }
    virtual std::string getValue() const override { return Config::getIntOr("MuteMusic", 0) ? "No" : "Yes"; }
    
    virtual void ToggleUp() override
    {
        Config::setInt("MuteMusic", !Config::getIntOr("MuteMusic", 0));
        Mix_VolumeMusic(GetMusicVolume());
    }
    
    virtual void ToggleDown() override
    {
        Config::setInt("MuteMusic", !Config::getIntOr("MuteMusic", 0));
        Mix_VolumeMusic(GetMusicVolume());
    }
} MusicMuteSetting;

struct : SettingItem
{
    virtual std::string_view getName() const override { return "Bloom Strength"; }
    virtual std::string getValue() const override { return std::to_string(Config::getIntOr("BloomStrength", DefaultBloomStrength)); }
    
    virtual void ToggleUp() override
    {
        Config::setInt("BloomStrength", std::clamp<int>(Config::getIntOr("BloomStrength", DefaultBloomStrength) + 10, 0, 100));
        Renderer::UpdateBloomStrength();
    }
    
    virtual void ToggleDown() override
    {
        Config::setInt("BloomStrength", std::clamp<int>(Config::getIntOr("BloomStrength", DefaultBloomStrength) - 10, 0, 100));
        Renderer::UpdateBloomStrength();
    }
} BloomStrengthSetting;

struct : SettingItem
{
    virtual std::string_view getName() const override { return "CRT Curve"; }
    virtual std::string getValue() const override { return std::to_string(Config::getIntOr("CrtCurve", DefaultCrtCurve)); }
    
    virtual void ToggleUp() override
    {
        Config::setInt("CrtCurve", std::clamp<int>(Config::getIntOr("CrtCurve", DefaultCrtCurve) + 25, 0, 300));
        Renderer::UpdateCrt();
    }
    
    virtual void ToggleDown() override
    {
        Config::setInt("CrtCurve", std::clamp<int>(Config::getIntOr("CrtCurve", DefaultCrtCurve) - 25, 0, 300));
        Renderer::UpdateCrt();
    }
} CrtCurveSetting;

struct : SettingItem
{
    virtual std::string_view getName() const override { return "Phosphor Effect"; }
    virtual std::string getValue() const override { return Config::getIntOr("PhosphorEnabled", 1) ? "Yes" : "No"; }
    
    virtual void ToggleUp() override
    {
        bool yes = !Config::getIntOr("PhosphorEnabled", 1);
        Config::setInt("PhosphorEnabled", yes);
        Renderer::PhosphorEnabled(yes);
    }
    
    virtual void ToggleDown() override
    {
        bool yes = !Config::getIntOr("PhosphorEnabled", 1);
        Config::setInt("PhosphorEnabled", yes);
        Renderer::PhosphorEnabled(yes);
    }
} PhosphorEnabledSetting;

struct : SettingItem
{
    virtual std::string_view getName() const override { return "Scanlines Effect"; }
    virtual std::string getValue() const override { return Config::getIntOr("CrtScanlinesEnabled", 1) ? "Yes" : "No"; }
    
    virtual void ToggleUp() override
    {
        bool yes = !Config::getIntOr("CrtScanlinesEnabled", 1);
        Config::setInt("CrtScanlinesEnabled", yes);
        Renderer::UpdateCrt();
    }
    
    virtual void ToggleDown() override
    {
        bool yes = !Config::getIntOr("CrtScanlinesEnabled", 1);
        Config::setInt("CrtScanlinesEnabled", yes);
        Renderer::UpdateCrt();
    }
} CrtScanlinesEnabledSetting;

struct : SettingItem
{
    virtual std::string_view getName() const override { return "Chromatic Aberration"; }
    virtual std::string getValue() const override { return Config::getIntOr("CrtCAEnabled", 1) ? "Yes" : "No"; }
    
    virtual void ToggleUp() override
    {
        bool yes = !Config::getIntOr("CrtCAEnabled", 1);
        Config::setInt("CrtCAEnabled", yes);
        Renderer::UpdateCrt();
    }
    
    virtual void ToggleDown() override
    {
        bool yes = !Config::getIntOr("CrtCAEnabled", 1);
        Config::setInt("CrtCAEnabled", yes);
        Renderer::UpdateCrt();
    }
} CrtCAEnabledSetting;

struct : SettingItem
{
    virtual std::string_view getName() const override { return "Vignette Effect"; }
    virtual std::string getValue() const override { return Config::getIntOr("CrtVignetteEnabled", 1) ? "Yes" : "No"; }
    
    virtual void ToggleUp() override
    {
        bool yes = !Config::getIntOr("CrtVignetteEnabled", 1);
        Config::setInt("CrtVignetteEnabled", yes);
        Renderer::UpdateCrt();
    }
    
    virtual void ToggleDown() override
    {
        bool yes = !Config::getIntOr("CrtVignetteEnabled", 1);
        Config::setInt("CrtVignetteEnabled", yes);
        Renderer::UpdateCrt();
    }
} CrtVignetteEnabledSetting;


std::vector<SettingItem*> settings
{
    &FontSetting,
    &ColorSetting,
    &BloomStrengthSetting,
    &PhosphorEnabledSetting,
    &CrtCurveSetting,
    &CrtScanlinesEnabledSetting,
    &CrtCAEnabledSetting,
    &CrtVignetteEnabledSetting,
    &VSyncSetting,
    &MusicMuteSetting,
    &VolumeSetting,
    &VolumeSoundSetting,
    &VolumeMusicSetting,
    &CompressSavesSetting,
};

void Menu::DrawSettingsMenu()
{
    Renderer::CurrentBuffer = &Renderer::MenuText;
    
    Renderer::MenuText.DrawClear();
    
    DrawBorderSingle();
    
    Renderer::MenuText.DrawText(16, 1, TitleSettings);
    
    Renderer::MenuText.DrawLineText(2, 8, SettingsTop);
    
    DrawSettings(currentSettingsMenuItem, settings);
}
