#include "Game.h"
#include "Common.h"
#include "Input.h"
#include "Renderer.h"
#include "Command.h"
#include "SaveData.h"
#include "Config.h"

#include <stdexcept>

using enum dir_entry_type;
using enum hide_type;

extern int currentScreen;

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

void Game::Init()
{
    LoadData();
    LoadIntro();
    
    CommandLineDrawPath = Config::getIntOr("CommandLineDrawPath", 1);
}

std::vector<std::string> Game::ListProgramsAt(std::string drive, std::string path, bool allow_hidden)
{
    auto entries = directories[drive].find(path);
    
    std::vector<std::string> programs;
    if(entries != directories[drive].end())
    {
        for(auto &entry : entries->second)
        {
            if((entry.second.type == PROGRAM || entry.second.type == PROGRAM_ALIAS) && (entry.second.hidden == VISIBLE || (allow_hidden && entry.second.hidden == HIDDEN)))
            {
                programs.push_back(entry.second.name);
            }
        }
    }
    return programs;
}

std::vector<std::string> Game::ListExecutablePrograms()
{
    return ListProgramsAt("C", "\\BIN\\", true);//Util::ConcatInplace(ListProgramsAt(currentDrive, currentFolder), ListProgramsAt("C", "\\BIN\\", true));
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

bool Game::HasAccess(const std::string &path_str, const std::string &command_name, std::string *finalPath, dir_entry **final_entry, dir_entry_type last_allowed, hide_type last_allowed_hide, bool allow_last_missing, bool silent, bool allow_gone)
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
                    if(entry == entries->second.end() || (entry->second.hidden == HIDDEN || entry->second.hidden == DELETED || (entry->second.hidden == GONE && !allow_gone)))
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
    if(currentScreen != 3) return;
    
    Renderer::GameText.HighRes();
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
                    continue;
                }
                
                if(action.info == "666")
                {
                    directories["C"]["\\BIN\\"].insert({action.info, {action.info, PROGRAM, HIDDEN}});
                }
                else
                {
                    directories["C"]["\\BIN\\"].insert({action.info, {action.info, PROGRAM}});
                }
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
                if(entries == directories[currentDrive].end()) continue;
                
                auto entry = directories[currentDrive][path].find(file);
                if(entry == entries->second.end()) continue;
                
                auto &e = directories[currentDrive][path][file];
                
                if(action.type == SaveData::RECOVERY)
                {
                    if(e.hidden == DELETED || e.hidden == CORRUPTED)
                    {
                        e.hidden = VISIBLE;
                    }
                }
                else if(action.type == SaveData::UNLOCK)
                {
                    if(e.hidden == ENCRYPTED && action.extra_info == e.password)
                    {
                        e.hidden = VISIBLE;
                    }
                }
                else if(action.type == SaveData::DELETE)
                {
                    e.hidden = GONE;
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

bool Game::CommandLineDrawPath = true;

void Game::Tick()
{
    int commandStart = 1;
    int freeCommandChars = 77;
    
    Renderer::DrawMenu = false;
    Renderer::DrawGame = true;
    
    Renderer::CurrentBuffer = &Renderer::GameText;
    Renderer::GameText.DrawClear();
    uint32_t offsetY = 1;
    
    size_t maxLines = std::min<size_t>(MaxConsoleLines, GameConsoleOutput.size());
    size_t offset = GameConsoleOutput.size() - maxLines;
    
    for(size_t i = 0; i < maxLines; i++)
    {
        if(GameConsoleOutput[offset + i].first.size() > 0)
        {
            Renderer::GameText.DrawLineText(1, offsetY, GameConsoleOutput[offset + i].first);
        }
        if(GameConsoleOutput[offset + i].second.size() > 0)
        {
            Renderer::GameText.DrawLineProp(1, offsetY, GameConsoleOutput[offset + i].second);
        }
        offsetY++;
    }
    
    if(CommandLineDrawPath)
    {
        std::string path = currentDrive + ":" + ((currentFolder.size() > 1) ? currentFolder.substr(0, currentFolder.size() - 1) : currentFolder); // strip last '/'
        
        Renderer::GameText.DrawLineTextFillProp(1, offsetY, path, 0);
        commandStart += path.size();
        freeCommandChars -= path.size();
    }
    
    int diff = (ssize_t(tempCommandPos) - ssize_t(tempCommandViewOffset));
    
    if(diff > (freeCommandChars - 3) && int(tempCommand.size()) > freeCommandChars)
    {
        tempCommandViewOffset = tempCommandPos - (freeCommandChars - 3);
    }
    
    if(diff < 0)
    {
        tempCommandViewOffset = tempCommandPos;
    }
    
    Renderer::GameText.DrawLineTextFillProp(commandStart, offsetY, ">", 0);
    
    if(tempCommandViewOffset > 1)
    {
        Renderer::GameText.DrawLineTextFillProp(commandStart + 1, offsetY, "<", CHAR_INVERT1);
        Renderer::GameText.DrawLineTextFillProp(commandStart + 2, offsetY, std::string_view(tempCommand.data() + tempCommandViewOffset, freeCommandChars - 2), 0);
        if(tempCommandViewOffset != tempCommand.size() - (freeCommandChars - 3)) // if last char isn't visible, overwrite it with a '>' arrow
        {
            Renderer::GameText.DrawLineTextFillProp(78, offsetY, ">", CHAR_INVERT1);
        }
    }
    else if(int(tempCommand.size()) > freeCommandChars)
    {
        Renderer::GameText.DrawLineTextFillProp(commandStart + 1, offsetY, std::string_view(tempCommand.data(), freeCommandChars - 1), 0);
        Renderer::GameText.DrawLineTextFillProp(78, offsetY, ">", CHAR_INVERT1); // last char before 'border'
    }
    else
    {
        Renderer::GameText.DrawLineTextFillProp(commandStart + 1, offsetY, tempCommand.data(), 0);
    }
    Renderer::GameText.DrawFillLineProp((tempCommandViewOffset > 0 ? commandStart + 2 : commandStart + 1) + (tempCommandPos - tempCommandViewOffset), offsetY, CHAR_UNDERSCORE, 1);
}
