#include "Common.h"
#include "Config.h"
#include "Json.h"

#include <filesystem>

JSON::Element configData;

bool config_ok = false;

constexpr const char * configFile = "config.json";

void Config::Init()
{
    if(std::filesystem::exists(configFile))
    {
        try
        {
            configData = JSON::Parse(Util::ReadFile(configFile));
            configData.get_obj();
        }
        catch(JSON::JSON_Exception &e)
        {
            throw FatalError("Malformed JSON in Config: "+e.msg_top);
        }
    }
    else
    {
        configData = JSON::Object({});
    }
    
    config_ok = true;
}

void Config::Quit()
{
    if(config_ok)
    {
        Util::WriteFile(configFile, configData.to_json());
    }
}

std::string * Config::getStringOrNull(const std::string &key)
{
    if(configData.get_obj().contains(key))
    {
        return &configData.get_obj()[key].get_str();
    }
    else
    {
        return nullptr;
    }
}

std::string_view Config::mustGetString(const std::string &key)
{
    if(configData.get_obj().contains(key))
    {
        return configData.get_obj()[key].get_str();
    }
    else
    {
        throw FatalError("Missing config key "+Util::QuoteString(key));
    }
}

std::string_view Config::getStringOr(const std::string &key, std::string_view alternative)
{
    if(configData.get_obj().contains(key))
    {
        return configData.get_obj()[key].get_str();
    }
    else
    {
        return alternative;
    }
}

int64_t Config::getIntOr(const std::string &key, int64_t alternative)
{
    if(configData.get_obj().contains(key))
    {
        return configData.get_obj()[key].get_int();
    }
    else
    {
        return alternative;
    }
}

std::string_view Config::setString(const std::string &key, std::string_view newValue)
{
    configData.get_obj()[key] = std::string(newValue);
    return newValue;
}

int64_t Config::setInt(const std::string &key, int64_t newValue)
{
    configData.get_obj()[key] = newValue;
    return newValue;
}
