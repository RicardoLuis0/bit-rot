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

struct SettingItem
{
    virtual ~SettingItem() = default;
    virtual std::string_view getName() const = 0;
    virtual std::string getValue() const = 0;
    virtual void Select() { ToggleUp(); }
    virtual void ToggleUp() = 0;
    virtual void ToggleDown() = 0;
    virtual bool hasValue() { return true; }
    virtual bool isSeparator() { return false; }
};

struct SettingButton : SettingItem
{
    std::string name;
    
    std::function<void()> onSelect;
    
    SettingButton(const std::string &_name, std::function<void()> _onSelect)
    : name(_name),
      onSelect(_onSelect)
    {}
    
    virtual std::string_view getName() const override
    {
        return name;
    }
    
    virtual std::string getValue() const override
    {
        return "";
    }
    
    virtual void Select()
    {
        onSelect();
    }
    
    virtual void ToggleUp() {};
    virtual void ToggleDown() {};
    
    virtual bool hasValue() { return false; }
};


struct SettingSeparator : SettingItem
{
    std::string name;
    
    SettingSeparator(const std::string &_name)
    : name(_name)
    {}
    
    virtual std::string_view getName() const override
    {
        return name;
    }
    
    virtual std::string getValue() const override
    {
        return "";
    }
    
    virtual void ToggleUp() {};
    virtual void ToggleDown() {};
    
    virtual bool hasValue() { return false; }
    virtual bool isSeparator() { return true; }
};

struct SettingItemYesNo : SettingItem
{
    std::string name;
    std::string configName;
    
    std::function<void(bool)> onUpdate;
    
    bool defaultValue;
    bool invertValue;
    
    SettingItemYesNo(const std::string &_name, const std::string &_configName, std::function<void(bool)> _onUpdate, bool _defaultValue, bool _invertValue = false)
    : name(_name),
      configName(_configName),
      onUpdate(_onUpdate),
      defaultValue(_defaultValue),
      invertValue(_invertValue)
    {}
    
    virtual std::string_view getName() const override
    {
        return name;
    }
    
    virtual std::string getValue() const override
    {
        return (Config::getBoolOr(configName, defaultValue) != invertValue) ? "Yes" : "No";
    }
    
    inline void Toggle()
    {
        bool newValue = !Config::getBoolOr(configName, defaultValue);
        Config::setBool(configName, newValue);
        onUpdate(newValue);
    }
    
    virtual void ToggleUp() override { Toggle(); }
    virtual void ToggleDown() override { Toggle(); }
};

struct SettingItemSlider : SettingItem
{
    std::string name;
    std::string configName;
    
    std::function<void(int)> onUpdate;
    
    int defaultValue;
    int minValue;
    int maxValue;
    int stepValue;
    
    bool invertStep;
    bool isPercent;
    
    SettingItemSlider(const std::string &_name, const std::string &_configName, std::function<void(bool)> _onUpdate, int _defaultValue, int _minValue, int _maxValue, int _stepValue, bool _isPercent = true, bool _invertStep = false)
    : name(_name),
      configName(_configName),
      onUpdate(_onUpdate),
      defaultValue(_defaultValue),
      minValue(_minValue),
      maxValue(_maxValue),
      stepValue(_stepValue),
      invertStep(_invertStep),
      isPercent(_isPercent)
    {}
    
    virtual std::string_view getName() const override
    {
        return name;
    }
    
    virtual std::string getValue() const override
    {
        return std::to_string(Config::getIntOr(configName, defaultValue)) + (isPercent ? "%" : "");
    }
    
    inline void doStep(bool direction)
    {
        int newValue = std::clamp<int>(Config::getIntOr(configName, defaultValue) + (direction? stepValue : -stepValue), minValue, maxValue);
        Config::setInt(configName, newValue);
        onUpdate(newValue);
    }
    
    virtual void ToggleUp() override
    {
        doStep(!invertStep);
    }
    
    virtual void ToggleDown() override
    {
        doStep(invertStep);
    }
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
    if(Config::getBoolOr("MuteMusic", DefaultMuteMusic)) return 0;
    
    double globalVol = Config::getIntOr("Volume", DefaultGlobalVolume) / 100.0;
    double musicVol = Config::getIntOr("VolumeMusic", DefaultMusicVolume) / 100.0;
    double vol = globalVol * musicVol;
    
    return (vol * vol) * 64; // limit "max" volume to 50% of absolute max
}

extern std::vector<SettingItem*> mainSettings;

std::vector<std::vector<SettingItem*>*> currentSettings;
std::vector<int> currentItem;
std::vector<int> currentScroll;

size_t maxSettings = 15;

constexpr int SettingsColumn1Width = 28;
constexpr int SettingsColumn2Width = 49;

void Menu::SettingsMenuResponder(SDL_Event *e)
{
    if(currentSettings.size() == 0)
    {
        currentSettings.push_back(&mainSettings);
        currentItem.push_back(1);
        currentScroll.push_back(0);
    }
    
    int &currentSettingsMenuItem = currentItem.back();
    int &currentSettingsMenuScroll = currentScroll.back();
    std::vector<SettingItem*> &settings = *currentSettings.back();
    
    switch(e->type)
    {
    case SDL_KEYDOWN:
        if(e->key == SDLK_ESCAPE)
        {
            if(currentSettings.size() <= 1)
            {
                currentScreen = Game::IsInGame() ? 2 : 0; // 2 = pause menu, 0 = main menu
                
                Renderer::ResetTimer();
                currentItem.pop_back();
                currentSettings.pop_back();
                currentScroll.pop_back();
            }
            else
            {
                Renderer::ResetTimer();
                currentItem.pop_back();
                currentSettings.pop_back();
                currentScroll.pop_back();
            }
        }
        else if(e->key == SDLK_UP || e->key == SDLK_DOWN)
        {
            if(e->key == SDLK_UP)
            {
                Renderer::ResetTimer();
                do
                {
                    currentSettingsMenuItem--;
                    if(currentSettingsMenuItem < 0) currentSettingsMenuItem = (settings.size() - 1);
                }
                while(settings[currentSettingsMenuItem]->isSeparator());
            }
            else if(e->key == SDLK_DOWN)
            {
                Renderer::ResetTimer();
                do
                {
                    currentSettingsMenuItem = (currentSettingsMenuItem + 1) % settings.size();
                }
                while(settings[currentSettingsMenuItem]->isSeparator());
            }
            
            if(currentSettingsMenuScroll > (currentSettingsMenuItem - 2)) currentSettingsMenuScroll = std::max(0, currentSettingsMenuItem - 2);
            if((currentSettingsMenuScroll + int(maxSettings - 3)) <= currentSettingsMenuItem) currentSettingsMenuScroll = std::min<int>(currentSettingsMenuItem - (maxSettings - 3), settings.size() - maxSettings);
            
            LogDebug("currentSettingsMenuScroll = "+std::to_string(currentSettingsMenuScroll)+" currentSettingsMenuItem = "+std::to_string(currentSettingsMenuItem));
        }
        else if(e->key == SDLK_RETURN)
        {
            settings[currentSettingsMenuItem]->Select();
        }
        else if(e->key == SDLK_RIGHT)
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

void DrawSetting(bool selected, std::string_view name, std::string_view value, int &y, bool start, bool end, bool hasValue, bool prevHasValue, bool isSeparator, bool prevWasSeparator, int width1, int width2)
{
    int fullWidth = (width1 + width2) - 1;
    //int x = ((80 - fullWidth) / 2);
    int x = 3;
    
    if(start || prevWasSeparator)
    {
        if(isSeparator)
        {
            // nothing
        }
        else if(hasValue)
        {
            // ┌─────
            //       ┬────────────┐
            // ┌─────┬────────────┐
            Menu::DrawHalfLine(x, y - 1, width1 - 1, BorderTop[0], BorderTop[1], 0);
            Menu::DrawLine(x + (width1 - 1), y - 1, width2, BorderTopEnd[0], BorderTopEnd[1], BorderTopEnd[2], 0);
        }
        else
        {
            // ┌──────────────────┐
            Menu::DrawLine(x, y - 1, fullWidth, BorderTop[0], BorderTop[1], BorderTop[2], 0);
        }
    }
    else if(isSeparator)
    {
        if(prevWasSeparator)
        {
            // nothing
        }
        if(prevHasValue)
        {
            // └─────
            //       ┴────────────┘
            // └─────┴────────────┘
            Menu::DrawHalfLine(x, y - 1, width1 - 1, BorderBottom[0], BorderBottom[1], 0);
            Menu::DrawLine(x + (width1 - 1), y - 1, width2, BorderBottomEnd[0], BorderBottomEnd[1], BorderBottomEnd[2], 0);
        }
        else
        {
            // └──────────────────┘
            Menu::DrawLine(x, y - 1, fullWidth, BorderBottom[0], BorderBottom[1], BorderBottom[2], 0);
        }
    }
    else
    {
        if(hasValue)
        {
            if(prevHasValue)
            {
                // ├─────
                //       ┼────────────┤
                // ├─────┼────────────┤
                Menu::DrawHalfLine(x, y - 1, width1 - 1, BorderSep[0], BorderSep[1], 0);
                Menu::DrawLine(x + (width1 - 1), y - 1, width2, BorderSepEnd[0], BorderSepEnd[1], BorderSepEnd[2], 0);
            }
            else
            {
                // ├─────
                //       ┬────────────┤
                // ├─────┬────────────┤
                Menu::DrawHalfLine(x, y - 1, width1 - 1, BorderSep[0], BorderSep[1], 0);
                Menu::DrawLine(x + (width1 - 1), y - 1, width2, BorderTopEnd[0], BorderSepEnd[1], BorderSepEnd[2], 0);
            }
        }
        else
        {
            if(prevHasValue)
            {
                // ├─────
                //       ┴────────────┤
                // ├─────┴────────────┤
                Menu::DrawHalfLine(x, y - 1, width1 - 1, BorderSep[0], BorderSep[1], 0);
                Menu::DrawLine(x + (width1 - 1), y - 1, width2, BorderBottomEnd[0], BorderSepEnd[1], BorderSepEnd[2], 0);
            }
            else
            {
                // ├──────────────────┤
                Menu::DrawLine(x, y - 1, fullWidth, BorderSep[0], BorderSep[1], BorderSep[2], 0);
            }
        }
    }
    
    if(isSeparator)
    {
        // nothing
    }
    else if(hasValue)
    {
        // │     
        //       │            │
        // │     │            │
        Menu::DrawHalfLine(x, y, width1 - 1, BorderMid[0], BorderMid[1], 0);
        Menu::DrawLine(x + (width1 - 1), y, width2, BorderMid[0], BorderMid[1], BorderMid[2], 0);
    }
    else
    {
        // │                  │
        Menu::DrawLine(x, y, fullWidth, BorderMid[0], BorderMid[1], BorderMid[2], 0);
    }
    
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
    
    if(end && !isSeparator)
    {
        if(hasValue)
        {
            // └─────
            //       ┴────────────┘
            // └─────┴────────────┘
            Menu::DrawHalfLine(x, y + 1, width1 - 1, BorderBottom[0], BorderBottom[1], 0);
            Menu::DrawLine(x + (width1 - 1), y + 1, width2, BorderBottomEnd[0], BorderBottomEnd[1], BorderBottomEnd[2], 0);
        }
        else
        {
            // └──────────────────┘
            Menu::DrawLine(x, y + 1, fullWidth, BorderBottom[0], BorderBottom[1], BorderBottom[2], 0);
        }
    }
    
    y += 2;
}

void DrawSettings(unsigned selection, const std::vector<SettingItem*> &settings, size_t start, size_t maxDraw, int width1, int width2)
{
    //size_t last = settings.size() - 1;
    int y = 9;
    bool prevHasValue = false;
    bool prevWasSeparator = false;
    
    size_t end = std::min(settings.size(), start + maxDraw);
    
    size_t last = end - 1;
    
    for(size_t i = start; i < end; i++)
    {
        DrawSetting(i == selection, settings[i]->getName(), settings[i]->getValue(), y, i == start, i == last, settings[i]->hasValue(), prevHasValue, settings[i]->isSeparator(), prevWasSeparator, width1, width2);
        
        prevHasValue = settings[i]->hasValue();
        prevWasSeparator = settings[i]->isSeparator();
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

SettingItemYesNo CompressSavesSetting("Compress Saves", "CompressSaves", [](bool){}, DefaultCompressSaves);

SettingItemSlider VolumeSetting("Global Volume", "Volume", [](int){Mix_MasterVolume(GetSoundVolume()); Mix_VolumeMusic(GetMusicVolume());}, DefaultGlobalVolume, 0, 100, 10);

SettingItemSlider VolumeSoundSetting("Sound Volume", "VolumeSound", [](int){Mix_MasterVolume(GetSoundVolume());}, DefaultSoundVolume, 0, 100, 10);

SettingItemSlider VolumeMusicSetting("Music Volume", "VolumeMusic", [](int){Mix_VolumeMusic(GetMusicVolume());}, DefaultMusicVolume, 0, 100, 10);

SettingItemYesNo MusicMuteSetting("Music", "MuteMusic", [](bool){Mix_VolumeMusic(GetMusicVolume());}, DefaultMuteMusic, true);

std::vector<SettingItem*> soundSettings
{
    new SettingSeparator("Volume Settings"),
    &VolumeSetting,
    &VolumeSoundSetting,
    &VolumeMusicSetting,
};

SettingButton SoundSettingsMenu("Volume", [](){
    Renderer::ResetTimer();
    currentSettings.push_back(&soundSettings);
    currentItem.push_back(1);
    currentScroll.push_back(0);
});

SettingItemSlider BloomStrengthSetting("Bloom Strength", "BloomStrength", [](int){Renderer::UpdateBloomStrength();}, DefaultBloomStrength, 0, 100, 10);

SettingItemSlider CrtCurveSetting("CRT Curve", "CrtCurve", [](int){Renderer::UpdateCrt();}, DefaultCrtCurve, 0, 300, 25);

SettingItemYesNo PhosphorEnabledSetting("Phosphor Effect", "PhosphorEnabled", [](bool yes){Renderer::PhosphorEnabled(yes);}, DefaultPhosphorEnabled);

SettingItemYesNo CrtScanlinesEnabledSetting("Scanlines Effect", "CrtScanlinesEnabled", [](bool){Renderer::UpdateCrt();}, DefaultCrtScanlinesEnabled);

SettingItemYesNo CrtCAEnabledSetting("Chromatic Aberration", "CrtCAEnabled", [](bool){Renderer::UpdateCrt();}, DefaultCrtCAEnabled);

SettingItemYesNo CrtVignetteEnabledSetting("Vignette Effect", "CrtVignetteEnabled", [](bool){Renderer::UpdateCrt();}, DefaultCrtVignetteEnabled);

SettingItemYesNo CrtPauseBlurEnabledSetting("Transparent Pause Menu", "CrtPauseBlurEnabled", [](bool){Renderer::UpdateCrt();}, DefaultCrtPauseBlurEnabled);

SettingItemYesNo ShowFPSSetting("Show FPS", "ShowFPS", [](bool on){Renderer::SetShowFPS(on);}, DefaultShowFPS);

std::vector<SettingItem*> graphicsSettings
{
    new SettingSeparator("Graphics Settings"),
    &BloomStrengthSetting,
    &PhosphorEnabledSetting,
    &CrtCurveSetting,
    &CrtScanlinesEnabledSetting,
    &CrtCAEnabledSetting,
    &CrtVignetteEnabledSetting,
    &CrtPauseBlurEnabledSetting,
};

SettingButton GraphicsSettingsMenu("Graphics Settings", [](){
    Renderer::ResetTimer();
    currentSettings.push_back(&graphicsSettings);
    currentItem.push_back(1);
    currentScroll.push_back(0);
});

SettingItemYesNo CommandLineDrawPathSetting("Show Path in Prompt", "CommandLineDrawPath", [](bool yes){Game::CommandLineDrawPath = yes;}, DefaultCommandLineDrawPath);

//SettingButton ButtonTestSetting("Test Button Long String Test Test Test", [](){});

extern int currentMainMenuItem;

SettingButton ClearSavesButton("Clear Saved Data", [](){
    SaveData::Clear();
    Config::setScriptString("SawIntro1", "no");
    currentMainMenuItem = 0;
});

std::vector<SettingItem*> debugSettings
{
    new SettingSeparator("Debug Settings"),
    &CompressSavesSetting,
    &ShowFPSSetting,
    &ClearSavesButton,
};

std::vector<SettingItem*> debugSettingsInGame
{
    new SettingSeparator("Debug Settings"),
    &CompressSavesSetting,
    &ShowFPSSetting,
};

SettingButton DebugSettingsMenu("Debug Settings", [](){
    Renderer::ResetTimer();
    currentSettings.push_back(Game::IsInGame()? &debugSettingsInGame : &debugSettings);
    currentItem.push_back(1);
    currentScroll.push_back(0);
});

std::vector<SettingItem*> mainSettings
{
    new SettingSeparator("Sound Settings"),
    &MusicMuteSetting,
    &SoundSettingsMenu,
    new SettingSeparator("Video Settings"),
    &FontSetting,
    &ColorSetting,
    &VSyncSetting,
    &GraphicsSettingsMenu,
    new SettingSeparator("Misc Settings"),
    &CommandLineDrawPathSetting,
    &DebugSettingsMenu,
};

void Menu::DrawSettingsMenu()
{
    if(currentSettings.size() == 0) return;
    
    int &currentSettingsMenuItem = currentItem.back();
    int &currentSettingsMenuScroll = currentScroll.back();
    std::vector<SettingItem*> &settings = *currentSettings.back();
    
    Renderer::CurrentBuffer = &Renderer::MenuText;
    
    Renderer::MenuText.DrawClear();
    
    DrawBorderSingle();
    
    Renderer::MenuText.DrawText(16, 1, TitleSettings);
    
    bool scroll = (settings.size() > maxSettings);
    
    int width2 = scroll ? SettingsColumn2Width - 3 : SettingsColumn2Width;
    
    DrawSettings(currentSettingsMenuItem, settings, currentSettingsMenuScroll, maxSettings, SettingsColumn1Width, width2);
    
    if(scroll)
    {
        DrawBorderSingle(0, 76, 8, 3, 31);
        //24, arrow up
        if(currentSettingsMenuScroll > 0) Renderer::CurrentBuffer->DrawChar(77, 8, 24, 0);
        //25, arrow down
        if(currentSettingsMenuScroll < (int(settings.size()) - int(maxSettings))) Renderer::CurrentBuffer->DrawChar(77, 38, 25, 0);
        
        int barSize = 29;
        double barPos = double(currentSettingsMenuScroll) / settings.size();
        double barPercent = double(maxSettings) / settings.size();
        
        int indicatorPos = std::max(9, int(barSize * barPos) + 9);
        int indicatorSize = std::min(int(barSize * barPercent), 28);
        int indicatorEnd = std::min(indicatorPos + indicatorSize, 37);
        
        //219, filled
        for(int y = indicatorPos; y <= indicatorEnd; y++)
        {
            Renderer::CurrentBuffer->DrawChar(77, y, 219, 0);
        }
    }
}
