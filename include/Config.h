#pragma once
#include <string>
#include <string_view>
#include <stdexcept>

#include "Common.h"

constexpr bool DefaultCompressSaves = true;
constexpr int DefaultGlobalVolume = 50;
constexpr int DefaultBloomStrength = 100;
constexpr int DefaultCrtCurve = 100;
constexpr int DefaultSoundVolume = 100;
constexpr int DefaultMusicVolume = 100;
constexpr bool DefaultMuteMusic = false;
constexpr bool DefaultPhosphorEnabled = true;
constexpr bool DefaultCrtScanlinesEnabled = true;
constexpr bool DefaultCrtCAEnabled = true;
constexpr bool DefaultCrtVignetteEnabled = false;
constexpr bool DefaultCrtPauseBlurEnabled = true;
constexpr bool DefaultCommandLineDrawPath = true;

namespace Config
{
    
    void Init();
    void Quit();
    
    /*
    std::string_view getSelectedFont();
    void setSelectedFont(std::string_view);
    */
    std::string * getStringOrNull(const std::string &key);
    
    
    
    
    
    std::string_view mustGetString(const std::string &key);
    std::string_view getStringOr(const std::string &key, std::string_view alternative);
    
    std::string_view getScriptStringOr(const std::string &key, std::string_view alternative);
    
    int64_t getIntOr(const std::string &key, int64_t alternative);
    
    bool getBoolOr(const std::string &key, bool alternative);
    
    std::string_view setString(const std::string &key, std::string_view newValue);
    
    std::string_view setScriptString(const std::string &key, std::string_view newValue);
    
    int64_t setInt(const std::string &key, int64_t newValue);
    
    bool setBool(const std::string &key, bool newValue);
    
    template<typename T, Util::ContainerComparableTo<std::string> C>
    T getEnumOr(const std::string &key, C &&values, T alternative)
    {
        std::string * val = getStringOrNull(key);
        if(val) for(unsigned i = 0; i < std::size(values); i++)
        {
            if(values[i] == *val) return T(i);
        }
        return alternative;
    }
    
    template<typename T, Util::ContainerConvertibleTo<std::string_view> C>
    void setEnum(const std::string &key, C &&values, T newValue)
    {
        if(size_t(newValue) >= std::size(values))
        {
            throw FatalError("Enum out of Range");
        }
        setString(key, values[size_t(newValue)]);
    }
}
