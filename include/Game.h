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
    GONE      = 0x00, // deleted, for good
    VISIBLE   = 0x01,
    CORRUPTED = 0x02,
    ENCRYPTED = 0x04,
    DELETED   = 0x08, // deleted, recoverable
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
    
    template<Util::ConvertibleTo<std::string_view> T, Util::ConvertibleTo<std::string_view> V>
    program_help(T && help_s, V && usage_str)
    {
        help = std::string(help_s);
        
        usage.push_back(std::string(usage_str));
    }
    
    template<Util::ConvertibleTo<std::string_view> T, Util::ContainerConvertibleTo<std::string_view> V>
    program_help(T && help_s, V && usage_arr)
    {
        help = std::string(help_s);
        
        for(auto &usage_str : usage_arr)
        {
            usage.push_back(std::string(usage_str));
        }
    }
    
    template<Util::ConvertibleTo<std::string_view> T, Util::ConvertibleTo<std::string_view> V, size_t N>
    program_help(T && help_s, const V (&usage_arr)[N])
    {
        help = std::string(help_s);
        
        for(auto &usage_str : usage_arr)
        {
            usage.push_back(std::string(usage_str));
        }
    }
    
    
    std::string help;
    std::vector<std::string> usage;
    
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
    
    extern bool CommandLineDrawPath;
    
    const constexpr size_t MaxConsoleLines = 37;
    
    void DoLoad();
    
    void LoadIntro();
    void ToIntro();
    void ToGame();
    
    std::vector<std::string> ListProgramsAt(std::string drive, std::string path, bool allow_hidden = false);
    std::vector<std::string> ListExecutablePrograms();
    bool HasAccess(const std::string &path_str, const std::string &command_name, std::string *finalPath = nullptr,
                   dir_entry **final_entry = nullptr, dir_entry_type last_allowed = dir_entry_type::ENTRY_ANY, hide_type last_allowed_hide = hide_type::HIDE_ANY,
                   bool allow_last_missing = false, bool silent = false, bool allow_gone = false);
    
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
