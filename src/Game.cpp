#include "Game.h"
#include "Common.h"
#include "Input.h"
#include "Renderer.h"
#include "Command.h"
#include "SaveData.h"

#include <stdexcept>

using enum dir_entry_type;
using enum hide_type;

extern int currentScreen;
extern int introStage;
extern uint32_t introStartMs;
extern uint32_t nextLineMs;

extern uint32_t memIncrement;

extern uint32_t lastIncrementMs;
extern uint32_t memAmount;

namespace Game
{
    std::string tempCommand;
    size_t tempCommandPos = 0;
    size_t tempCommandViewOffset = 0;
    
    std::string tempCommandPreHistory;
    std::vector<std::string> commandHistory;
    ssize_t historyPos = -1;
    
    std::vector<std::pair<std::string, std::vector<uint8_t>>> GameConsoleOutput;
    
    bool GameIsSave = false;
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
    GameIsSave = true;
}


void Game::ToGame()
{
    currentScreen = 4;
    GameConsoleOutput.clear();
    if(GameIsSave)
    {
        std::vector<SaveData::SaveAction> actions;
        
        SaveData::GetSave(currentFolder, actions, commandHistory, GameConsoleOutput);
        
        for(auto &action : actions)
        {
            if(action.type == SaveData::INSTALL)
            {
                if(programs.find(action.info) == programs.end())
                {
                    continue; //throw std::runtime_error("Bad Save Data");
                }
                directories["C"]["\\BIN\\"].insert({action.info, {action.info, PROGRAM}});
            }
            else
            {
                std::string path;
                std::string file;
                size_t split = action.info.find_last_of('\\');
                if(split == std::string::npos)
                {
                    path = "\\";
                    file = action.info;
                }
                else
                {
                    path = action.info.substr(0, split + 1);
                    file = action.info.substr(split + 1);
                }
                auto entries = directories[currentDrive].find(path);
                if(entries == directories[currentDrive].end()) continue; //throw std::runtime_error("Bad Save Data");
                
                auto entry = directories[currentDrive][path].find(file);
                if(entry == entries->second.end()) continue; //throw std::runtime_error("Bad Save Data");
                
                auto &e = directories[currentDrive][path][file];
                
                if(action.type == SaveData::RECOVERY)
                {
                    if(e.hidden == DELETED || e.hidden == CORRUPTED)
                    {
                        e.hidden = VISIBLE;
                    }
                    /*
                    else
                    {
                        throw std::runtime_error("Bad Save Data");
                    }
                    */
                }
                else if(action.type == SaveData::UNLOCK)
                {
                    
                    if(e.hidden == ENCRYPTED && action.extra_info == e.password)
                    {
                        e.hidden = VISIBLE;
                    }
                    /*
                    else
                    {
                        throw std::runtime_error("Bad Save Data");
                    }
                    */
                }
            }
        }
        
    }
    else
    {
        SaveData::Reset();
        AddConsoleLine("RD-OS v6.66 RECOVERY MODE", {0,0,0,0,0,0,0,0,0,0,0,0,CHAR_INVERT1,CHAR_INVERT1,CHAR_INVERT1,CHAR_INVERT1,CHAR_INVERT1,CHAR_INVERT1,CHAR_INVERT1,CHAR_INVERT1,CHAR_INVERT1,CHAR_INVERT1,CHAR_INVERT1,CHAR_INVERT1,CHAR_INVERT1});
        AddConsoleLine("");
        AddConsoleLine("  Type 'HELP' for help");
        AddConsoleLine("");
        SaveData::MarkNewGameOk();
    }
}

void Game::Tick()
{
    Renderer::DrawClear();
    uint32_t offsetY = 1;
    
    size_t maxLines = std::min<size_t>(MaxConsoleLines, GameConsoleOutput.size());
    size_t offset = GameConsoleOutput.size() - maxLines;
    
    for(size_t i = 0; i < maxLines; i++)
    {
        if(GameConsoleOutput[offset + i].first.size() > 0)
        {
            Renderer::DrawLineText(1, offsetY, GameConsoleOutput[offset + i].first);
        }
        if(GameConsoleOutput[offset + i].second.size() > 0)
        {
            Renderer::DrawLineProp(1, offsetY, GameConsoleOutput[offset + i].second);
        }
        offsetY++;
    }
    
    int diff = (ssize_t(tempCommandPos) - ssize_t(tempCommandViewOffset));
    
    if(diff > 74 && tempCommand.size() > 77)
    {
        tempCommandViewOffset = tempCommandPos - 74;
    }
    
    if(diff < 0)
    {
        tempCommandViewOffset = tempCommandPos;
    }
    
    Renderer::DrawLineTextFillProp(1, offsetY, ">", 0);
    
    if(tempCommandViewOffset > 1)
    {
        Renderer::DrawLineTextFillProp(2, offsetY, "<", CHAR_INVERT1);
        Renderer::DrawLineTextFillProp(3, offsetY, std::string_view(tempCommand.data() + tempCommandViewOffset, 75), 0);
        if(tempCommandViewOffset != tempCommand.size() - 74)
        {
            Renderer::DrawLineTextFillProp(78, offsetY, ">", CHAR_INVERT1);
        }
    }
    else if(tempCommand.size() > 77)
    {
        Renderer::DrawLineTextFillProp(2, offsetY, std::string_view(tempCommand.data(), 76), 0);
        Renderer::DrawLineTextFillProp(78, offsetY, ">", CHAR_INVERT1);
    }
    else
    {
        Renderer::DrawLineTextFillProp(2, offsetY, tempCommand.data(), 0);
    }
    Renderer::DrawFillLineProp((tempCommandViewOffset > 0 ? 3 : 2) + (tempCommandPos - tempCommandViewOffset), offsetY, CHAR_UNDERSCORE, 1);
}
