#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <map>
#include <vector>

union SDL_Event;

using CommandProc = void(*)(const std::vector<std::string>&);


enum class dir_entry_type : uint8_t
{
    FOLDER    = 0x01,
    TEXT      = 0x02,
    DATA      = 0x04,
    PROGRAM   = 0x08,
    DRIVER    = 0x10,
    //
    ENTRY_ANY = 0xFF,
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

extern std::map<std::string, CommandProc> programs;
extern std::string currentDrive;
extern std::string currentFolder;
extern std::map<std::string, std::string> textFiles;
extern std::map<std::string, std::string> textFilesCorrupted;
extern std::map<std::string, std::map<std::string, std::map<std::string, dir_entry>>> directories;

namespace Game
{
    void DoLoad();
    inline void DoSave() {} // TODO
    inline bool HasSave() {return false;} // TODO
    
    void ToIntro();
    void ToGame();
    
    std::vector<std::string> ListProgramsAt(std::string drive, std::string path);
    std::vector<std::string> ListPrograms();
    bool HasAccess(const std::string &path_str, const std::string &command_name, std::string *finalPath = nullptr, dir_entry **final_entry = nullptr, dir_entry_type last_allowed = dir_entry_type::ENTRY_ANY, hide_type last_allowed_hide = hide_type::HIDE_ANY, bool allow_last_missing = false);
    
    void IntroResponder(SDL_Event *e);
    void Responder(SDL_Event *e);
    void EndResponder(SDL_Event *e);
    void TickIntro();
    void Tick();
    void TickEnd();
    
    void ClearConsole();
    void AddConsoleLine(const std::string &text);
    void AddConsoleLine(const std::string &text, const std::vector<uint8_t> &props);
    
    
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
};
