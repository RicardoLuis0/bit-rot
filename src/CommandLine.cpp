#include "Common.h"
#include "Game.h"
#include "Input.h"
#include "SaveData.h"
#include "SDL2Util.h"

extern int currentScreen;

using enum dir_entry_type;
using enum hide_type;

bool lastWasTab = false;

std::vector<dir_entry> ListFolders(std::string path)
{
    std::string folder;
    if(Game::HasAccess(path, "", &folder, nullptr, FOLDER, VISIBLE, false, true))
    {
        return Game::ListDir(folder);
    }
    else
    {
        return {};
    }
}


static const std::string &DirName(const dir_entry& e)
{
    return e.name;
}

void Game::Responder(SDL_Event *e)
{
    switch(e->type)
    {
    case SDL_KEYDOWN:
        if(e->key != SDLK_TAB)
        {
            lastWasTab = false;
        }
        
        if(e->key == SDLK_ESCAPE)
        {
            currentScreen = 2; // pause
        }
        else if(e->key == SDLK_UP)
        {
            bool isMax = tempCommandPos == tempCommand.size();
            if(historyPos == -1)
            {
                tempCommandPreHistory = tempCommand;
            }
            historyPos++;
            tempCommand = commandHistory[historyPos];
            tempCommandPos = isMax ? tempCommand.size() : std::min(tempCommandPos, tempCommand.size());
        }
        else if(e->key == SDLK_DOWN)
        {
            bool isMax = tempCommandPos == tempCommand.size();
            if(historyPos > 0)
            {
                historyPos--;
                tempCommand = commandHistory[historyPos];
                tempCommandPos = isMax ? tempCommand.size() : std::min(tempCommandPos, tempCommand.size());
            }
            else if(historyPos == 0)
            {
                historyPos--;
                tempCommand = tempCommandPreHistory;
                tempCommandPos = isMax ? tempCommand.size() : std::min(tempCommandPos, tempCommand.size());
            }
            else
            {
                tempCommandPos = tempCommand.size();
            }
        }
        else if(e->key == SDLK_RIGHT)
        {
            if(tempCommandPos < tempCommand.size())
            {
                if(Input::CtrlPressed())
                {
                    size_t p = tempCommand.find_first_of(' ', tempCommandPos + 1);
                    if(p != std::string::npos)
                    {
                        tempCommandPos = p + 1;
                    }
                    else
                    {
                        tempCommandPos = tempCommand.size();
                    }
                }
                else
                {
                    tempCommandPos++;
                }
            }
        }
        else if(e->key == SDLK_LEFT)
        {
            if(tempCommandPos > 0)
            {
                if(Input::CtrlPressed())
                {
                    size_t p = tempCommand.find_last_of(' ', tempCommandPos - ((tempCommand[tempCommandPos - 1] == ' ' && (tempCommandPos - 2) > 0) ? 2 : 1));
                    if(p != std::string::npos)
                    {
                        tempCommandPos = p;
                    }
                    else
                    {
                        tempCommandPos = 0;
                    }
                }
                else
                {
                    tempCommandPos--;
                }
            }
        }
        else if(e->key == SDLK_BACKSPACE && tempCommandPos > 0 && tempCommand.size() > 0)
        {
            tempCommandPos--;
            if(tempCommandViewOffset > 0) tempCommandViewOffset--;
            tempCommand.erase(tempCommandPos, 1);
            historyPos = -1;
            tempCommandPreHistory = "";
        }
        else if(e->key == SDLK_RETURN)
        {
            std::string command = tempCommand;
            tempCommandPos = 0;
            tempCommand = "";
            RunCommand(command);
            historyPos = -1;
            tempCommandPreHistory = "";
        }
        else if(e->key == SDLK_TAB)
        {
            std::vector<Util::SplitPoint> args = Util::SplitStringEx(tempCommand, ' ', true, true);
            
            const Util::SplitPoint * currentArg = nullptr;
            const Util::SplitPoint * newArg = &*args.end();
            
            for(auto arg = args.begin(); arg < args.end(); arg++)
            {
                if(arg->offset > tempCommandPos) break;
                currentArg = &*arg;
            }
            
            if(currentArg != nullptr && currentArg == args.data() + (args.size() - 1) && tempCommandPos > (currentArg->offset + currentArg->orig_len))
            { // past the end treat as "new" arg
                currentArg = newArg;
            }
            
            if(currentArg == nullptr || currentArg == newArg || currentArg->to_view().empty())
            {
                if(lastWasTab)
                {
                    std::string alts = "";
                    if(currentArg == nullptr || currentArg == args.data() || (currentArg == newArg && args.size() == 1 && Util::StrToUpper(args[0].to_view()) == "HELP"))
                    {
                        alts = Util::Join(ListPrograms(), " ");
                    }
                    else
                    {
                        alts = Util::Join(Util::Map(ListFolders(currentFolder), DirName), " ");
                    }
                    lastWasTab = false;
                    Game::AddConsoleLine(">"+tempCommand);
                    Util::ForEach(Util::SplitLines((Input::ShiftPressed() || (e->key.keysym.mod & KMOD_CAPS)) ? alts : Util::StrToLower(alts), 78), (void(*)(std::string_view))Game::AddConsoleLine);
                }
                else
                {
                    lastWasTab = true;
                }
                break;
            }
            else if(currentArg == args.data() || (currentArg == args.data() + 1 && Util::StrToUpper(args[0].to_view()) == "HELP"))
            { // program name, TODO dehardcode it from help (leave type handling per command) if any new commands take non-paths
                std::string arg = Util::StrToUpper(currentArg->to_view());
                std::vector<std::string> completionAlternatives =
                                                Util::Filter(ListPrograms(),
                                                        std::bind((bool(std::string::*)(std::string_view)const noexcept)&std::string::starts_with, std::placeholders::_1, std::string_view(arg)));
                if(completionAlternatives.size() > 1)
                {
                    if(lastWasTab)
                    {
                        std::string alts = Util::Join(completionAlternatives, " ");
                        lastWasTab = false;
                        Game::AddConsoleLine(">"+tempCommand);
                        Util::ForEach(Util::SplitLines((Input::ShiftPressed() || (e->key.keysym.mod & KMOD_CAPS)) ? alts : Util::StrToLower(alts), 78), (void(*)(std::string_view))Game::AddConsoleLine);
                    }
                    else
                    {
                        lastWasTab = true;
                    }
                }
                else if(completionAlternatives.size() == 1)
                {
                    if(completionAlternatives[0] != arg)
                    {
                        tempCommand.replace(currentArg->offset, currentArg->orig_len, (Input::ShiftPressed() || (e->key.keysym.mod & KMOD_CAPS)) ? completionAlternatives[0] : Util::StrToLower(completionAlternatives[0]));
                        tempCommandPos = currentArg->offset + completionAlternatives[0].size();
                        if(currentArg == args.data() + (args.size() - 1))
                        {
                            tempCommand += " ";
                            tempCommandPos++;
                        }
                    }
                    else if(tempCommandPos == currentArg->offset + currentArg->orig_len && currentArg == args.data() + (args.size() - 1))
                    {
                        tempCommand += " ";
                        tempCommandPos++;
                    }
                }
            }
            else
            { // path
                std::vector<std::string> path_vec = Util::MapInplace(Util::SplitString(currentArg->to_view(), "\\/", false, false), Util::StrToUpper);
                std::string path = "";
                std::string partial = "";
                if(path_vec.size() == 0 || (path_vec.size() == 1 && path_vec[0].empty()))
                {
                    break;
                }
                if(currentArg->to_view().back() == '\\' || currentArg->to_view().back() == '/')
                {
                    path = Util::Join(path_vec, "/");
                    partial = "";
                }
                else if(path_vec.size() > 0)
                {
                    if(path_vec.size() > 1)
                    {
                        path = Util::Join(std::span(path_vec.begin(), path_vec.end() -1), "/");
                    }
                    partial = path_vec.back();
                }
                
                std::vector<dir_entry> completionAlternatives = Util::Filter(ListFolders(path), [p = std::string_view(partial)](dir_entry &e){return e.name.starts_with(p);});
                
                if(completionAlternatives.size() > 1)
                {
                    if(lastWasTab)
                    {
                        std::string alts = Util::Join(Util::Map(completionAlternatives, DirName), " ");
                        lastWasTab = false;
                        Game::AddConsoleLine(">"+tempCommand);
                        Util::ForEach(Util::SplitLines((Input::ShiftPressed() || (e->key.keysym.mod & KMOD_CAPS)) ? alts : Util::StrToLower(alts), 78), (void(*)(std::string_view))Game::AddConsoleLine);
                    }
                    else
                    {
                        lastWasTab = true;
                    }
                }
                else if(completionAlternatives.size() == 1)
                {
                    std::string alt = (path.empty() ? path : path + "/") + completionAlternatives[0].name;
                    if(completionAlternatives[0].name != partial)
                    {
                        tempCommand.replace(currentArg->offset, currentArg->orig_len, (Input::ShiftPressed() || (e->key.keysym.mod & KMOD_CAPS)) ? alt : Util::StrToLower(alt));
                        tempCommandPos = currentArg->offset + alt.size();
                    }
                    else if(tempCommandPos == currentArg->offset + currentArg->orig_len && currentArg == args.data() + (args.size() - 1) && completionAlternatives[0].type == FOLDER)
                    {
                        tempCommand += "/";
                        tempCommandPos++;
                    }
                }
            }
        }
        else if(e->key.keysym.sym >= ' ' && e->key.keysym.sym <= '~')
        {
            char c = e->key.keysym.sym;
            if(Input::ShiftPressed() || (e->key.keysym.mod & KMOD_CAPS))
            {
                c = Util::CharToUpper(c);
            }
            tempCommand.insert(tempCommandPos, std::string(1, c));
            tempCommandPos++;
            historyPos = -1;
            tempCommandPreHistory = "";
        }
        break;
    }
}

void Game::RunCommand(const std::string &command, bool isQueue)
{
    std::vector<std::string> commandQueue = Util::SplitString(command, ';', true, true, true);
    
    if(commandQueue.size() > 1)
    {
        commandHistory.insert(commandHistory.begin(), command);
        SaveData::PushHistory(command);
        
        for(const std::string &cmd : commandQueue)
        {
            RunCommand(cmd, true);
            if(currentScreen != 4) break;
        }
        
    }
    else
    {
        std::vector<std::string> programsList = Game::ListPrograms();
        
        
        
        std::vector<std::string> args = Util::SplitString(command, ' ', true, true);
        if(args.size() > 0)
        {
            std::string cmd = Util::StrToUpper(args[0]);
            
            if(args.size() > 1 || (cmd != "EXIT" && cmd != "666")) // TODO: stop muting 666 after expanding the story
            {
                Game::AddConsoleLine(">"+command);
                if(!isQueue)
                {
                    commandHistory.insert(commandHistory.begin(), command);
                    SaveData::PushHistory(command);
                }
            }
            
            if(std::find(programsList.begin(), programsList.end(), cmd) != programsList.end())
            {
                programs[cmd](args);
            }
            else
            {
                Game::AddConsoleLine("");
                Game::AddConsoleLine("Could not find program "+Util::QuoteString(cmd));
                Game::AddConsoleLine("");
            }
        }
    }
}
