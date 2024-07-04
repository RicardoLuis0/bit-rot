#pragma once
#include <string>
#include <string_view>
#include <stdexcept>

#include "Common.h"

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
    
    int64_t getIntOr(const std::string &key, int64_t alternative);
    
    std::string_view setString(const std::string &key, std::string_view newValue);
    
    int64_t setInt(const std::string &key, int64_t newValue);
    
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
