#pragma once

#include "Common.h"

#include <string>
#include <vector>
#include <cstdint>
#include <map>
#include <vector>

union SDL_Event;

using CommandProc = void(*)(const std::vector<std::string>&);


enum class dir_entry_type : uint8_t
{
    FOLDER          = 0x01,
    TEXT            = 0x02,
    DATA            = 0x04,
    PROGRAM         = 0x08,
    PROGRAM_ALIAS   = 0x10,
    DRIVER          = 0x20,
    //
    ENTRY_ANY       = 0xFF,
};

enum class hide_type : uint8_t
{
    VISIBLE   = 0x01,
    CORRUPTED = 0x02,
    ENCRYPTED = 0x04,
    DELETED   = 0x08,
    HIDDEN    = 0x10,
    FORBIDDEN = 0x20,
    //for HasAccess only
    HIDE_ANY  = 0xFF,
};

struct dir_entry
{
    std::string name;
    dir_entry_type type;
    hide_type hidden = hide_type::VISIBLE;
    std::string password = "";
};

using line = std::pair<std::string_view, std::vector<uint8_t>>;

struct program_help
{
    program_help() = default;
    
    #define __BOTH_CTR(help_type, help_args, util_type, util_args) program_help( PP_DEPAREN(help_type) , PP_DEPAREN(util_type) ) : PP_DEPAREN(help_args) , PP_DEPAREN(util_args) {}
    
    #define BOTH_CTR(help, util) PP_INDIRECT(__BOTH_CTR , PP_DEPAREN(help) , PP_DEPAREN(util))
    
    PP_FOREACH_PAIRS(BOTH_CTR,
               ((std::string_view help_str), (help({{std::string(help_str), {}}}))),
               ((std::string_view usage_str), (usage({{std::string(usage_str), {}}}))),
               ((std::string_view help_str, const std::vector<uint8_t> &help_props), (help({{std::string(help_str), help_props}}))),
               ((std::string_view usage_str, const std::vector<uint8_t> &usage_props), (usage({{std::string(usage_str), usage_props}}))),
               ((std::string_view help_str, std::vector<uint8_t> &&help_props), (help({{std::string(help_str), std::move(help_props)}}))),
               ((std::string_view usage_str, std::vector<uint8_t> &&usage_props), (usage({{std::string(usage_str), std::move(usage_props)}})))
    );
    
    #undef BOTH_CTR
    #undef __BOTH_CTR
    
    #define __USAGE_CTR(args, help_args) template<size_t N> program_help PP_DEPAREN(args)\
    {\
        for(auto &usage_str : usage_strs)\
        {\
            usage.push_back help_args;\
        }\
    }
    
    #define USAGE_CTR(help) PP_INDIRECT(__USAGE_CTR , PP_DEPAREN(help) )
    
    #define __USAGE_FOREACH( type, args ) PP_FOREACH(USAGE_CTR, \
               (((std::string_view help_str, PP_DEPAREN(type) ) : help({{std::string(help_str), {}}})), args), \
               (((std::string_view help_str, const std::vector<uint8_t> &help_props, PP_DEPAREN(type) ) : help({{std::string(help_str), help_props}})), args), \
               (((std::string_view help_str, std::vector<uint8_t> &&help_props, PP_DEPAREN(type) ) : help({{std::string(help_str), std::move(help_props)}})), args) \
    )
    
    #define USAGE_FOREACH(args) PP_INDIRECT2(__USAGE_FOREACH , PP_DEPAREN(args) )
    
    PP_FOREACH2(USAGE_FOREACH,
               ((const std::string_view (&usage_strs)[N]), ({std::string(usage_str), {}})),
               ((const std::pair<std::string_view, std::vector<uint8_t>> (&usage_strs)[N]), ({std::string(usage_str.first), usage_str.second})),
               ((std::pair<std::string_view, std::vector<uint8_t>> (&&usage_strs)[N]), ({std::string(usage_str.first), std::move(usage_str.second)}))
    )
    
    #undef __USAGE_CTR
    #undef USAGE_CTR
    #undef __USAGE_FOREACH
    #undef USAGE_FOREACH
    
    
    
    #define __HELP_CTR(args, help_args) template<size_t N> program_help args\
    {\
        for(auto &help_str : help_strs)\
        {\
            help.push_back help_args;\
        }\
    }
    
    #define HELP_CTR(help) PP_INDIRECT(__HELP_CTR , PP_DEPAREN(help) )
    
    #define __HELP_FOREACH( type, args ) PP_FOREACH(HELP_CTR, \
               ((PP_DEPAREN(type) , std::string_view usage_str) : usage({{std::string(usage_str), {}}}), args), \
               ((PP_DEPAREN(type) , std::string_view usage_str, const std::vector<uint8_t> &usage_props) : usage({{std::string(usage_str), usage_props}}), args), \
               ((PP_DEPAREN(type) , std::string_view usage_str, std::vector<uint8_t> &&usage_props) : usage({{std::string(usage_str), std::move(usage_props)}}), args) \
    )\
    
    #define HELP_FOREACH(args) PP_INDIRECT2(__HELP_FOREACH , PP_DEPAREN(args) )
    
    PP_FOREACH2(HELP_FOREACH,
        ((const std::string_view (&help_strs)[N]), ({std::string(help_str), {}})),
        ((const std::pair<std::string_view, std::vector<uint8_t>> (&help_strs)[N]), ({std::string(help_str.first), help_str.second})),
        ((std::pair<std::string_view, std::vector<uint8_t>> (&&help_strs)[N]), ({std::string(help_str.first), std::move(help_str.second)}))
    )
    
    #undef __HELP_CTR
    #undef HELP_CTR
    #undef __HELP_FOREACH
    #undef HELP_FOREACH
    
    #define __BOTH_CTR(help_type, help_args, util_type, util_args) template<size_t N1, size_t N2> program_help( PP_DEPAREN(help_type) , PP_DEPAREN(util_type) ) \
    {\
        for(auto &help_str : help_strs)\
        {\
            help.push_back help_args ;\
        }\
        for(auto &usage_str : usage_strs)\
        {\
            usage.push_back util_args;\
        }\
    }
    
    #define BOTH_CTR(help, util) PP_INDIRECT(__BOTH_CTR , PP_DEPAREN(help) , PP_DEPAREN(util))
    
    PP_FOREACH_PAIRS(BOTH_CTR,
               ((const std::string_view (&help_strs)[N1]), ({std::string(help_str), {}})),
               ((const std::string_view (&usage_strs)[N2]), ({std::string(usage_str), {}})),
               ((const std::pair<std::string_view, std::vector<uint8_t>> (&help_strs)[N1]), ({std::string(help_str.first), help_str.second})),
               ((const std::pair<std::string_view, std::vector<uint8_t>> (&usage_strs)[N2]), ({std::string(usage_str.first), usage_str.second})),
               ((std::pair<std::string_view, std::vector<uint8_t>> (&&help_strs)[N1]), ({std::string(help_str.first), std::move(help_str.second)})),
               ((std::pair<std::string_view, std::vector<uint8_t>> (&&usage_strs)[N2]), ({std::string(usage_str.first), std::move(usage_str.second)}))
    );
    
    #undef BOTH_CTR
    #undef __BOTH_CTR
    
    std::vector<std::pair<std::string, std::vector<uint8_t>>> help;
    std::vector<std::pair<std::string, std::vector<uint8_t>>> usage;
    
    bool hidden = false;
    
    static program_help&& hide(program_help &&h)
    {
        h.hidden = true;
        return std::move(h);
    }
};

struct initTextLine
{
    uint32_t timer;
    std::string text;
    bool beep = false;
    bool recovery = true;
    bool intro = true;
};

extern std::map<std::string, CommandProc> programs;
extern std::string currentDrive;
extern std::string currentFolder;
extern std::map<std::string, std::string> textFiles;
extern std::map<std::string, std::string> textFilesCorrupted;
extern std::map<std::string, std::string> texts;
extern std::map<std::string, std::map<std::string, std::map<std::string, dir_entry>>> directories;
extern std::map<std::vector<std::string>, program_help> programHelp;

extern std::vector<initTextLine> initText;
extern uint32_t numRecoveryTexts;

namespace Game
{
    void Init();
    inline void Quit() {};
    
    extern std::string tempCommand;
    extern size_t tempCommandPos;
    extern size_t tempCommandViewOffset;
    
    extern std::string tempCommandPreHistory;
    extern std::vector<std::string> commandHistory;
    extern ssize_t historyPos;
    
    extern std::vector<std::pair<std::string, std::vector<uint8_t>>> GameConsoleOutput;
    
    extern bool GameIsSave;
    
    const constexpr size_t MaxConsoleLines = 37;
    
    void DoLoad();
    
    void ToIntro();
    void ToGame();
    
    std::vector<std::string> ListProgramsAt(std::string drive, std::string path);
    std::vector<std::string> ListPrograms();
    bool HasAccess(const std::string &path_str, const std::string &command_name, std::string *finalPath = nullptr,
                   dir_entry **final_entry = nullptr, dir_entry_type last_allowed = dir_entry_type::ENTRY_ANY, hide_type last_allowed_hide = hide_type::HIDE_ANY,
                   bool allow_last_missing = false, bool silent = false);
    
    std::vector<dir_entry> ListDir(const std::string &folder, size_t * max_len = nullptr);
    
    void IntroResponder(SDL_Event *e);
    void Responder(SDL_Event *e);
    void EndResponder(SDL_Event *e);
    void TickIntro();
    void Tick();
    void TickEnd();
    void RunCommand(const std::string &cmd, bool isQueue = false);
    
    void ClearConsole();
    void AddConsoleLine(std::string_view text);
    void AddConsoleLine(std::string_view text, const std::vector<uint8_t> &props);
    
    
    inline void AddConsoleLines(const std::vector<std::string> &text)
    {
        for(const std::string &line : text)
        {
            AddConsoleLine(line);
        }
    }
    
    inline void AddConsoleLines(const std::vector<std::pair<std::string, std::vector<uint8_t>>> &text)
    {
        for(const auto &line : text)
        {
            AddConsoleLine(line.first, line.second);
        }
    }
    
    void LoadData();
};
