#include "Game.h"
#include "Common.h"
#include "Input.h"
#include "Renderer.h"
#include "Command.h"


using enum dir_entry_type;
using enum hide_type;

extern int currentScreen;
extern int introStage;
extern uint32_t introStartMs;

extern uint32_t memIncrement;

extern uint32_t lastIncrementMs;
extern uint32_t memAmount;

namespace Game
{
    std::string tempCommand;
    size_t tempCommandPos = 0;
    
    std::string tempCommandPreHistory;
    std::vector<std::string> commandHistory;
    ssize_t historyPos = -1;
    
    std::vector<std::pair<std::string, std::vector<uint8_t>>> GameConsoleOutput;
}

std::vector<std::string> Game::ListProgramsAt(std::string drive, std::string path)
{
    auto entries = directories[drive].find(path);
    
    std::vector<std::string> programs;
    if(entries != directories[drive].end())
    {
        for(auto &entry : entries->second)
        {
            if((entry.second.type == PROGRAM || entry.second.type == PROGRAM_ALIAS) && entry.second.hidden == VISIBLE)
            {
                programs.push_back(entry.second.name);
            }
        }
    }
    return programs;
}

std::vector<std::string> Game::ListPrograms()
{
    return ListProgramsAt("C", "\\BIN\\");//Util::ConcatInplace(ListProgramsAt(currentDrive, currentFolder), ListProgramsAt("C", "\\BIN\\"));
}

std::string prevPath(std::string path)
{
    if(path == "\\")
    {
        return path;
    }
    else if(path.size() > 1)
    {
        return path.substr(0, path.find_last_of('\\', path.size() - 2) + 1);
    }
    else
    {
        return "";
    }
}

std::string pathFolder(std::string path)
{
    if(path == "\\")
    {
        return path;
    }
    else if(path.size() > 1)
    {
        size_t last = path.find_last_of('\\', path.size() - 2) + 1;
        return path.substr(last, (path.size() - last) - 1);
    }
    else
    {
        return "";
    }
}

static inline void DoErr(std::string_view msg, std::string_view err)
{
    Game::AddConsoleLine(std::string(msg) + std::string(err), Util::Concat(std::vector<uint8_t>(msg.size(), 0), std::vector<uint8_t>(err.size(), CHAR_INVERT1)));
}

bool Game::HasAccess(const std::string &path_str, const std::string &command_name, std::string *finalPath, dir_entry **final_entry, dir_entry_type last_allowed, hide_type last_allowed_hide, bool allow_last_missing, bool silent)
{
    if(final_entry) *final_entry = nullptr;
    std::vector<std::string> path_vec = Util::MapInplace(Util::SplitString(path_str, "\\/", false, false), Util::StrToUpper);
    if(path_vec.size() > 0)
    {
        bool absolute = path_vec[0].empty();
        std::string path = absolute? "\\" : currentFolder;
        
        for(size_t i = absolute ? 1 : 0; i < path_vec.size(); i++)
        {
            auto entries = directories[currentDrive].find(path);
            
            if(entries == directories[currentDrive].end())
            {
                if(!silent) AddConsoleLine("Inexistent Path Passed to "+command_name);
                return false;
            }
            
            std::string tmppath;
            if(path_vec[i] == ".")
            {
                continue;
            }
            else if(path_vec[i] == "..")
            {
                tmppath = prevPath(path);
            }
            else
            {
                tmppath = path + path_vec[i] + "\\";
            }
            
            if(path_vec[i] != ".." || tmppath != "\\")
            {
                auto entry = path_vec[i] == ".." ? directories[currentDrive].find(prevPath(tmppath))->second.find(pathFolder(tmppath)) : entries->second.find(path_vec[i]);
                
                if(entry == entries->second.end() || entry->second.type != FOLDER)
                {
                    tmppath = prevPath(tmppath) + pathFolder(tmppath); // strip last slash for non-folders
                }
                
                if((!allow_last_missing && entry == entries->second.end())
                   || i < (path_vec.size() - 1) || (entry != entries->second.end() && i == (path_vec.size() - 1)
                       && (!(uint8_t(last_allowed) & uint8_t(entry->second.type))
                       || !(uint8_t(last_allowed_hide) & uint8_t(entry->second.hidden)))))
                {
                    if(entry == entries->second.end() || (entry->second.hidden == HIDDEN || entry->second.hidden == DELETED))
                    {
                        if(!silent) DoErr("Invalid Path "+Util::QuoteString(tmppath, '\'', false)+" Passed to "+command_name+": ",Util::QuoteString(path_vec[i], '\'', false)+" does not Exist");
                        return false;
                    }
                    else if(entry->second.type != FOLDER && ((i != (path_vec.size() - 1)) || last_allowed == FOLDER))
                    {
                        if(!silent) DoErr("Invalid Path "+Util::QuoteString(tmppath, '\'', false)+" Passed to "+command_name+": ",Util::QuoteString(path_vec[i], '\'', false)+" is not a Folder");
                        return false;
                    }
                    else if(entry->second.hidden == ENCRYPTED)
                    {
                        if(!silent) DoErr("Invalid Path "+Util::QuoteString(tmppath, '\'', false)+" Passed to "+command_name+": ", Util::QuoteString(path_vec[i], '\'', false)+" is Encrypted");
                        return false;
                    }
                    else if(entry->second.hidden == FORBIDDEN)
                    {
                        if(!silent) DoErr("Invalid Path "+Util::QuoteString(tmppath, '\'', false)+" Passed to "+command_name+": ","Access Forbidden");
                        return false;
                    }
                    else if(entry->second.hidden == CORRUPTED)
                    {
                        if(!silent)
                        {
                            uint32_t time = Util::MsTime();
                            
                            std::string msg = stringRand(stringRandReplace("Invalid Path "+Util::QuoteString(tmppath, '\'', false)+" Passed to "+command_name+": ",'_', 64, time + 1), '_', time + 2);
                            std::string err = stringRand(stringRandReplace("Error trying to Access "+Util::QuoteString(path_vec[i], '\'', false)+" is "+std::string(const_rand(time) % 10, '_'),'_', 64, time + 3), '_', time + 4);
                            
                            DoErr(msg, err);
                        }
                        return false;
                    }
                    else if(i == (path_vec.size() - 1))
                    {
                        if(!silent) AddConsoleLine("Invalid Path "+Util::QuoteString(tmppath, '\'', false)+" Passed to "+command_name);
                    }
                }
                else if(i == (path_vec.size() - 1) && final_entry)
                {
                    if(entry == entries->second.end())
                    {
                        *final_entry = nullptr;
                    }
                    else
                    {
                        *final_entry = &entry->second;
                    }
                }
            }
            path = tmppath;
        }
        
        if(finalPath) *finalPath = std::move(path);
        return true;
    }
    else
    {
        if(silent)
        {
            if(finalPath) *finalPath = currentFolder;
            return true;
        }
        else
        {
            AddConsoleLine("Empty Path Passed to "+command_name);
            return false;
        }
    }
}

void Game::DoLoad()
{
    introStage = 1;
    currentScreen = 3;
    introStartMs = Util::MsTime();
    Audio::StartFan();
}


void Game::ToGame()
{
    currentScreen = 4;
    GameConsoleOutput.clear();
    GameConsoleOutput.push_back({"RD-OS v6.66 RECOVERY MODE", {0,0,0,0,0,0,0,0,0,0,0,0,CHAR_INVERT1,CHAR_INVERT1,CHAR_INVERT1,CHAR_INVERT1,CHAR_INVERT1,CHAR_INVERT1,CHAR_INVERT1,CHAR_INVERT1,CHAR_INVERT1,CHAR_INVERT1,CHAR_INVERT1,CHAR_INVERT1,CHAR_INVERT1}});
    GameConsoleOutput.push_back({"",{}});
    GameConsoleOutput.push_back({"  Type 'HELP' for help",{}});
    GameConsoleOutput.push_back({"",{}});
}

void Game::Tick()
{
    Renderer::DrawClear();
    uint32_t offsetY = 1;
    size_t maxLines = std::min<size_t>(MaxConsoleLines, GameConsoleOutput.size());
    
    for(size_t i = 0; i < maxLines; i++)
    {
        if(GameConsoleOutput[i].first.size() > 0)
        {
            Renderer::DrawLineText(1, offsetY, GameConsoleOutput[i].first);
        }
        if(GameConsoleOutput[i].second.size() > 0)
        {
            Renderer::DrawLineProp(1, offsetY, GameConsoleOutput[i].second);
        }
        offsetY++;
    }
    Renderer::DrawLineTextFillProp(1, offsetY, ">", 0);
    Renderer::DrawLineTextFillProp(2, offsetY, tempCommand, 0);
    Renderer::DrawFillLineProp(2 + tempCommandPos, offsetY, CHAR_UNDERSCORE, 1);
}
