#include <SDL2/SDL.h>
#include <cstdarg>
#include <vector>
#include <bitset>
#include <mutex>
#include <ctime>
#include <cassert>


#include "Common.h"
#include "Log.h"

using namespace Log;

const char * LogCategoryStrings[]
{
    //built-in
    "APPLICATION",
    "ERROR",
    "ASSERT",
    "SYSTEM",
    "AUDIO",
    "VIDEO",
    "RENDER",
    "INPUT",
    "TEST",
    //reserved
    "RESERVED",
    "RESERVED",
    "RESERVED",
    "RESERVED",
    "RESERVED",
    "RESERVED",
    "RESERVED",
    "RESERVED",
    "RESERVED",
    "RESERVED",
    //custom
    "INVALID",
};

const char * LogPriorityStrings[]
{
    "INVALID",
    "VERBOSE",
    "DEBUG",
    "INFO",
    "WARN",
    "ERROR",
    "CRITICAL",
};

static_assert(SDL_LOG_CATEGORY_CUSTOM == 19);
static_assert((sizeof(LogCategoryStrings) / sizeof(*LogCategoryStrings)) == (int)LogCategory::COUNT);

bool log_inited = false;

namespace Log
{
    std::vector<int> filteredLogs;
    std::vector<log_item_t> Logs;
    
    
    std::bitset<(uint8_t)LogCategory::COUNT> FilteredCategories;
    std::bitset<(uint8_t)LogPriority::COUNT> FilteredPriorities;
    
    std::mutex logLock;
    
    std::string formatTimestamp(time_t timestamp)
    {
        char buffer[20];
        strftime(buffer, 19, "%H:%M:%S", localtime(&timestamp));
        return {buffer};
    }
    
    std::string formatMessage(const log_item_t & log, int formatFlags = 0)
    {
        std::string fmt = "[" + formatTimestamp(log.timestamp) + " - " + LogPriorityStrings[uint8_t(log.priority)];
        if(log.category == LogCategory::APPLICATION)
        {
            if(log.fn_namespace.size() > 0)
            {
                assert(log.fn_name.size() > 0);
                fmt += "] [" + log.fn_namespace + "::" + log.fn_name;
            }
            else if(log.fn_name.size() > 0)
            {
                fmt += "] [" + log.fn_name;
            }
            
            if(log.file_name.size() > 0)
            {
                fmt += "] [" + log.file_name + ":" + std::to_string(log.file_line);
            }
        }
        else
        {
            fmt += " - ";
            fmt += LogCategoryStrings[uint8_t(log.category)];
        }
        fmt += "] " + log.msg;
        return fmt;
    }
    
    void FilterLog(size_t i)
    {
        log_item_t &log = Logs[i];
        if(!(FilteredCategories[uint8_t(log.category)] || FilteredPriorities[uint8_t(log.priority)]))
        {
            filteredLogs.push_back(Logs.size() - 1);
            puts(formatMessage(log).c_str());
        }
    }
    
    void AddLog(log_item_t && orig_log)
    {
        std::lock_guard g(logLock);
        
        Logs.emplace_back(std::move(orig_log));
        
        if(log_inited) [[likely]]
        {
            FilterLog(Logs.size() - 1);
        }
    }
    
    void LogHandler(void *,int cat, SDL_LogPriority pri, const char * msg);
    
    void Init()
    {
        std::lock_guard g(logLock);
        
        FilteredPriorities[(uint8_t)LogPriority::VERBOSE] = true;
        
        for(size_t i = 0; i < Logs.size(); i++)
        {
            FilterLog(i);
        }
        
        log_inited = true;
        
        // it's fine to skip SDL_Main, it only does utf8 conversions
        SDL_SetMainReady();
        
        // redirect logs to our handler
        SDL_LogSetOutputFunction(&LogHandler, nullptr);
        
        for(int i = SDL_LOG_CATEGORY_APPLICATION; i < (int)LogCategory::COUNT; i++)
        {
            // enable all logs for all categories
            SDL_LogSetPriority(i, SDL_LOG_PRIORITY_VERBOSE);
        }
    }
    
    void Quit()
    {
        Util::WriteFile("info.log", Util::Join(Util::Map(Logs, std::bind(formatMessage, std::placeholders::_1, 0)), "\n"));
    }
    
    void LogHandler(void *,int cat, SDL_LogPriority pri, const char * msg)
    {
        time_t timestamp = time(NULL);
        if(cat < 0 || cat >= (int)LogCategory::COUNT) cat = (int)LogCategory::INVALID;
        if(pri < 0 || pri > SDL_LOG_PRIORITY_CRITICAL) pri = (SDL_LogPriority) LogPriority::INVALID;
        
        AddLog({{}, {}, {}, -1, msg, timestamp, (LogPriority)pri, (LogCategory)cat});
    }
    
    void LogFull(LogPriority priority, std::string_view fn_namespace, std::string_view fn_name, std::string_view file_name, int line, const char *fmt, ...)
    {
        time_t timestamp = time(NULL);
        
        VAStart(args, fmt);
        std::string msg = Util::VFormat(fmt, args);
        
        AddLog({std::string(fn_namespace), std::string(fn_name), std::string(file_name), line, std::move(msg), timestamp, priority, LogCategory::APPLICATION});
    }
    
    void LogFull(LogPriority priority, std::string_view fn_namespace, std::string_view fn_name, std::string_view file_name, int line, const std::string &msg)
    {
        time_t timestamp = time(NULL);
        AddLog({std::string(fn_namespace), std::string(fn_name), std::string(file_name), line, msg, timestamp, priority, LogCategory::APPLICATION});
    }
    
    void LogFull(LogPriority priority, std::string_view fn_namespace, std::string_view fn_name, std::string_view file_name, int line, std::string_view msg)
    {
        time_t timestamp = time(NULL);
        AddLog({std::string(fn_namespace), std::string(fn_name), std::string(file_name), line, std::string(msg), timestamp, priority, LogCategory::APPLICATION});
    }
    
    void Disable()
    {
        log_inited = false;
    }
}
