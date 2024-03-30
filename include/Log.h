#pragma once

#include <string>
#include <cstdint>
#include <vector>
#include <ctime>

enum class LogCategory : uint8_t
{
    APPLICATION = 0, // = SDL_LOG_CATEGORY_APPLICATION
    INVALID = 19, // = SDL_LOG_CATEGORY_CUSTOM,
    COUNT
};

enum class LogPriority : uint8_t
{
    INVALID,
    VERBOSE,
    DEBUG,
    INFO,
    WARN,
    ERROR,
    CRITICAL,
    COUNT
};

consteval int c_strlen(const char * str)
{
    int i = 0;
    for(;str[i] != '\0'; i++);
    return i;
}

consteval int full_funcname_start(const char * funcname)
{
    for(int i = 0; funcname[i] != 0; i++)
    {
        if(funcname[i] == ' ' && (i <= 0 || funcname[i - 1] != ',')) return i+1;
        if(funcname[i] == '(') return 0;
    }
    return 0;
}

consteval int full_funcname_end(const char * funcname)
{
    for(int i = 0; funcname[i] != 0; i++)
    {
        if(funcname[i] == '(') return i;
    }
}

consteval int full_funcname_len(const char * funcname)
{
    return full_funcname_end(funcname) - full_funcname_start(funcname);
}

consteval int funcname_start(const char * funcname)
{
    int i = full_funcname_start(funcname);
    
    int full_start = i;
    
    int last_colon = -10;
    
    for(; funcname[i] != 0; i++)
    {
        if(funcname[i] == '(')
        {
            break;
        }
        else
        if(funcname[i] == ':')
        {
            if(last_colon == (i - 1))
            {
                return i + 1;
            }
            else
            {
                last_colon = i;
            }
        }
    }
    
    return full_start; // no namespace
}

consteval int funcname_len(const char * funcname)
{
    return full_funcname_end(funcname) - funcname_start(funcname);
}

consteval int namespace_start(const char * funcname)
{
    return full_funcname_start(funcname);
}

consteval int namespace_len(const char * funcname)
{
    int len = funcname_start(funcname) - full_funcname_start(funcname);
    return len > 0 ? len - 2 : 0;
}

consteval int filename_start(const char * filename)
{
    int last_slash = -1;
    for(int i = 0; filename[i] != '\0'; i++)
    {
        if(filename[i] == '\\' || filename[i] == '/') last_slash = i;
    }
    return last_slash + 1;
}

#ifdef _MSC_VER
    #define LogLine(priority, fmt, ...)\
        Log::LogFull(priority, std::string_view(__FUNCSIG__ + namespace_start(__FUNCSIG__) , namespace_len(__FUNCSIG__)), std::string_view(__FUNCSIG__ + funcname_start(__FUNCSIG__) , funcname_len(__FUNCSIG__)), std::string_view(__FILE__ + filename_start(__FILE__)) , __LINE__ , fmt __VA_OPT__(,) __VA_ARGS__)
#elif defined(__GNUC__)
    #define LogLine(priority, fmt, ...)\
        Log::LogFull(priority, std::string_view(__PRETTY_FUNCTION__ + namespace_start(__PRETTY_FUNCTION__) , namespace_len(__PRETTY_FUNCTION__)), std::string_view(__PRETTY_FUNCTION__ + funcname_start(__PRETTY_FUNCTION__) , funcname_len(__PRETTY_FUNCTION__)), std::string_view(__FILE__ + filename_start(__FILE__)) , __LINE__ , fmt __VA_OPT__(,) __VA_ARGS__)
#else
    #define LogLine(priority, fmt, ...)\
        Log::LogFull(priority, {} , std::string_view(__func__), std::string_view(__FILE__ + filename_start(__FILE__)) , __LINE__ , fmt __VA_OPT__(,) __VA_ARGS__)
#endif
#define LogVerbose(fmt, ...)\
    LogLine(LogPriority::VERBOSE, fmt __VA_OPT__(,) __VA_ARGS__)
#define LogDebug(fmt, ...)\
    LogLine(LogPriority::DEBUG, fmt __VA_OPT__(,) __VA_ARGS__)
#define LogInfo(fmt, ...)\
    LogLine(LogPriority::INFO, fmt __VA_OPT__(,) __VA_ARGS__)
#define LogWarn(fmt, ...)\
    LogLine(LogPriority::WARN, fmt __VA_OPT__(,) __VA_ARGS__)
#define LogError(fmt, ...)\
    LogLine(LogPriority::ERROR, fmt __VA_OPT__(,) __VA_ARGS__)
#define LogCritical(fmt, ...)\
    LogLine(LogPriority::CRITICAL, fmt __VA_OPT__(,) __VA_ARGS__)

namespace Log
{
    struct log_item_t
    {
        std::string fn_namespace;
        std::string fn_name;
        std::string file_name;
        int file_line;
        
        std::string msg;
        time_t timestamp;
        LogPriority priority;
        LogCategory category;
    };
    
    extern std::vector<int> filteredLogs;
    extern std::vector<log_item_t> Logs;
    
    void Init();
    void Quit();
    
    void Disable();
    
    void LogFull(LogPriority priority, std::string_view fn_namespace, std::string_view fn_name, std::string_view file_name, int line, const std::string &msg);
    void LogFull(LogPriority priority, std::string_view fn_namespace, std::string_view fn_name, std::string_view file_name, int line, std::string_view msg);
    void LogFull(LogPriority priority, std::string_view fn_namespace, std::string_view fn_name, std::string_view file_name, int line, const char * fmt, ...);
};
