#include "Game.h"
#include "Common.h"
#include "Input.h"
#include "Renderer.h"
#include "SDL2Util.h"
#include "Command.h"

#include <map>

using enum dir_entry_type;
using enum hide_type;

extern int currentScreen;
extern int introStage;
extern uint32_t introStartMs;

extern uint32_t memIncrement;

extern uint32_t lastIncrementMs;
extern uint32_t memAmount;

std::string tempCommand;
size_t tempCommandPos = 0;

std::string currentDrive = "C";
std::string currentFolder = "\\";


//names

#define YOU "Sam"
#define SISTER "Abigail"

#define FATHER "Blair"
#define MOTHER "Noelle"

#define UNCLE "Kevin"
#define UNCLE_NICKNAME "Kev"

std::map<std::string, std::string> textFilesCorrupted
{
                              //"I think there's something wrong with this computer, "
    {"\\HOME\\DOCS\\BROKEN", fixStringRand(U"_______   there's something wrong with   _____________, "
                              //"ever since updating the OS, this weird driver started loading, '666.sys', "
                                "ever since   _______________,   this   _______________________________________, "
                              //"I can't find it on the '\\SYSTEM\\DRV\\' folder. I tried asking Kev if he knew more, " 
                                "I can't   ______________   '\\SYSTEM\\DRV\\'   _________   tried asking   _________   knew   ______"
                              //"but when i tried showing him, all he saw on the monitor was a blank screen..."
                                "___  when i tried  ____________________________________________  blank screen...", U'_', 123)},
};

std::map<std::string, std::string> textFiles
{
    {"\\HOME\\DOCS\\BROKEN", "I think there's something wrong with this computer, "
                 "ever since updating the   OS, this weird driver started loading, '666.sys'. "
                 "I can't find it on the      '\\SYSTEM\\DRV\\' folder. I tried asking Kev if he knew more, "
                 "but when i tried   showing him, all he saw on the monitor was a blank screen..."},
                 
    //{"\\DELETEME", ""},
    {"\\HOME\\README", "If i've forgotten my passwords, here's a hint: birthday"},
    {"\\HOME\\DOCS\\DATES", "Me - 24/07/67                                                                 "
                            "Noelle - 08/02/65                                                             "
                            "Abigail - 14/04/89                                                            "
    },
    {"\\HOME\\SENSITIVE\\IF_YOU_FOUND_THIS", "If you found this computer, please destroy it, i couldn't bear to             do it myself..."},
};

std::map<std::string, CommandProc> programs
{
    {"CD", &Command::Cd},
    {"DIR", &Command::Dir},
    {"LS", &Command::Dir},
    {"CLS", &Command::Clear},
    {"CLEAR", &Command::Clear},
    {"HELP", &Command::Help},
    //{"MAN", &Command::Help},
    {"READ", &Command::Read},
    {"CAT", &Command::Read},
    {"INSTALL", &Command::Install},
    {"UNLOCK", &Command::Unlock},
    {"RECOVERY", &Command::Recovery},
    {"DECRYPT", &Command::Decrypt},
    {"666", &Command::EndJamBuild},
};

std::map<std::string, std::map<std::string, std::map<std::string, dir_entry>>> directories
{
    {"C", {
        {"\\", {
            {"SYSTEM", {"SYSTEM", FOLDER}},
            {"BIN", {"BIN", FOLDER, FORBIDDEN}},
            {"HOME", {"HOME", FOLDER}},
            //{"DELETEME", {"DELETEME", TEXT, DELETED}},
        }},
        {"\\SYSTEM\\",{
            {"OS", {"OS", FOLDER, FORBIDDEN}},
            {"CFG", {"CFG", FOLDER, FORBIDDEN}},
            {"DRV", {"DRV", FOLDER}},
        }},
        {"\\SYSTEM\\OS\\",{
            
        }},
        {"\\SYSTEM\\CFG\\",{
            
        }},
        {"\\SYSTEM\\DRV\\",{
            {"PowerMgmt",{"PowerMgmt", DRIVER}},
            {"Display",{"Display", DRIVER}},
            {"Console",{"Console", DRIVER}},
            {"ExtMemory",{"ExtMemory", DRIVER}},
            {"SndBurst",{"SndBurst", DRIVER}},
            {"Command",{"Command", DRIVER}},
            {"Mouse",{"Mouse", DRIVER}},
            {"Printer",{"Printer", DRIVER}},
            {"Network",{"Network", DRIVER}},
            //{"666",{"666", DRIVER, HIDDEN}},
            {"666",{"666", PROGRAM, DELETED}},
        }},
        {"\\BIN\\",{
            {"CD", {"CD", PROGRAM}},
            {"CLS", {"CLS", PROGRAM}},
            {"CLEAR", {"CLEAR", PROGRAM}},
            {"DIR", {"DIR", PROGRAM}},
            {"LS", {"LS", PROGRAM}},
            {"HELP", {"HELP", PROGRAM}},
            //{"MAN", {"MAN", PROGRAM}},
            {"READ", {"READ", PROGRAM}},
            {"CAT", {"CAT", PROGRAM}},
            {"INSTALL", {"INSTALL", PROGRAM}},
            //{"UNLOCK", {"UNLOCK", PROGRAM}},
            //{"RECOVERY", {"RECOVERY", PROGRAM}},
            //{"DECRYPT", {"DECRYPT", PROGRAM}},
        }},
        {"\\HOME\\",{
            {"DOCS", {"DOCS", FOLDER}},
            {"SENSITIVE", {"SENSITIVE", FOLDER, ENCRYPTED, "140489"}},
            {"README", {"README", TEXT}},
        }},
        {"\\HOME\\DOCS\\",{
            {"BROKEN", {"BROKEN", TEXT, CORRUPTED}},
            {"DATES", {"DATES", TEXT}},
            {"UNLOCK", {"UNLOCK", PROGRAM}},
        }},
        {"\\HOME\\SENSITIVE\\",{
            {"IF_YOU_FOUND_THIS", {"IF_YOU_FOUND_THIS", TEXT}},
            {"RECOVERY", {"RECOVERY", PROGRAM}},
        }},
    }},
};

std::vector<std::string> Game::ListProgramsAt(std::string drive, std::string path)
{
    auto entries = directories[drive].find(path);
    
    std::vector<std::string> programs;
    if(entries != directories[drive].end())
    {
        for(auto &entry : entries->second)
        {
            if(entry.second.type == PROGRAM && entry.second.hidden == VISIBLE)
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

bool Game::HasAccess(const std::string &path_str, const std::string &command_name, std::string *finalPath, dir_entry **final_entry, dir_entry_type last_allowed, hide_type last_allowed_hide, bool allow_last_missing)
{
    if(final_entry) *final_entry = nullptr;
    std::vector<std::string> path_vec = Util::MapInplaceChain(Util::SplitString(path_str, "\\/", false, false), Util::StrToUpper);
    if(path_vec.size() > 0)
    {
        bool absolute = path_vec[0].empty();
        std::string path = absolute? "\\" : currentFolder;
        
        for(size_t i = absolute ? 1 : 0; i < path_vec.size(); i++)
        {
            auto entries = directories[currentDrive].find(path);
            
            if(entries == directories[currentDrive].end())
            {
                AddConsoleLine("Inexistent Path Passed to "+command_name);
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
                        AddConsoleLine("Invalid Path "+Util::QuoteString(tmppath, '\'', false)+" Passed to "+command_name+": "+Util::QuoteString(path_vec[i], '\'', false)+" does not Exist");
                        return false;
                    }
                    else if(entry->second.type != FOLDER && ((i != (path_vec.size() - 1)) || last_allowed == FOLDER))
                    {
                        AddConsoleLine("Invalid Path "+Util::QuoteString(tmppath, '\'', false)+" Passed to "+command_name+": "+Util::QuoteString(path_vec[i], '\'', false)+" is not a Folder");
                        return false;
                    }
                    else if(entry->second.hidden == ENCRYPTED)
                    {
                        AddConsoleLine("Invalid Path "+Util::QuoteString(tmppath, '\'', false)+" Passed to "+command_name+": "+Util::QuoteString(path_vec[i], '\'', false)+" is Encrypted");
                        return false;
                    }
                    else if(entry->second.hidden == FORBIDDEN)
                    {
                        AddConsoleLine("Invalid Path "+Util::QuoteString(tmppath, '\'', false)+" Passed to "+command_name+": "+Util::QuoteString(path_vec[i], '\'', false)+" Access Forbidden");
                        return false;
                    }
                    else if(entry->second.hidden == CORRUPTED)
                    {
                        uint32_t time = Util::MsTime();
                        AddConsoleLine(
                           stringRand(stringRandReplace("Invalid Path "+Util::QuoteString(tmppath, '\'', false)+" Passed to "+command_name+": Error trying to Access "+Util::QuoteString(path_vec[i], '\'', false)+" is "+std::string(const_rand(time) % 10, '_')
                            ,'_', 64, time + 1
                           ), '_', time + 2));
                        return false;
                    }
                    else if(i == (path_vec.size() - 1))
                    {
                        AddConsoleLine("Invalid Path "+Util::QuoteString(tmppath, '\'', false)+" Passed to "+command_name);
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
        AddConsoleLine("Empty Path Passed to "+command_name);
        return false;
    }
}


std::string tempCommandPreHistory;
std::vector<std::string> commandHistory;
ssize_t historyPos = -1;

std::vector<std::pair<std::string, std::vector<uint8_t>>> GameConsoleOutput;

char toUppercase(char c)
{
    //TODO uppercase
    if(c >= 'a' && c <= 'z')
    {
        return c + ('A' - 'a');
    }
    else switch(c)
    {
    case '\'': return '"';
    case '\\': return '|';
    case '/': return '?';
    case ';': return ':';
    case '.': return '>';
    case ',': return '<';
    case '[': return '{';
    case ']': return '}';
    case '-': return '_';
    case '=': return '+';
    case '1': return '!';
    case '2': return '@';
    case '3': return '#';
    case '4': return '$';
    case '5': return '%';
    case '6': return '^';
    case '7': return '&';
    case '8': return '*';
    case '9': return '(';
    case '0': return ')';
    }
    return c;
}

void runCommand(std::string command)
{
    Game::AddConsoleLine(">"+command);
    commandHistory.insert(commandHistory.begin(), command);
    
    std::vector<std::string> programsList = Game::ListPrograms();
    
    std::vector<std::string> args = Util::SplitString(command, ' ', true, true);
    std::string cmd = Util::StrToUpper(args[0]);
    
    if(std::find(programsList.begin(), programsList.end(), cmd) != programsList.end())
    {
        programs[cmd](args);
    }
    else
    {
        Game::AddConsoleLine("Could not find program "+Util::QuoteString(cmd));
    }
    /*
    if(cmd == "dir")
    {
        LogDebug("Dir Command, args: "+Util::JoinOr(Util::Map(args, std::bind(Util::QuoteString, std::placeholders::_1, '\'')), ", ", " and "));
        //TODO
    }
    else if(cmd == "cls" || cmd == "clear")
    {
        LogDebug("Cls Command, args: "+Util::JoinOr(Util::Map(args, std::bind(Util::QuoteString, std::placeholders::_1, '\'')), ", ", " and "));
        //TODO
    }
    */
}

void Game::Responder(SDL_Event *e)
{
    switch(e->type)
    {
    case SDL_KEYDOWN:
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
            tempCommand.erase(tempCommandPos, 1);
            historyPos = -1;
            tempCommandPreHistory = "";
        }
        else if(e->key == SDLK_RETURN)
        {
            std::string command = tempCommand;
            tempCommandPos = 0;
            tempCommand = "";
            runCommand(command);
            historyPos = -1;
            tempCommandPreHistory = "";
        }
        else if(e->key.keysym.sym >= ' ' && e->key.keysym.sym <= '~')
        {
            if(tempCommand.size() < 77)
            {
                char c = e->key.keysym.sym;
                if(Input::ShiftPressed())
                {
                    c = toUppercase(c);
                }
                tempCommand.insert(tempCommandPos, std::string(1, c));
                tempCommandPos++;
                historyPos = -1;
                tempCommandPreHistory = "";
            }
        }
        break;
    }
}

constexpr size_t MaxConsoleLines = 37;


static void InternalAddConsoleLine(std::string_view text, std::span<const uint8_t> props)
{
    if(GameConsoleOutput.size() == MaxConsoleLines)
    {
        GameConsoleOutput.erase(GameConsoleOutput.begin());
    }
    GameConsoleOutput.push_back({std::string(text), std::vector<uint8_t>(props.begin(), props.end())});
}

void Game::ClearConsole()
{
    GameConsoleOutput.clear();
}

void Game::AddConsoleLine(const std::string &text)
{
    AddConsoleLine(text, {});
}

void Game::AddConsoleLine(const std::string &_text, const std::vector<uint8_t> &_props)
{
    if(_text.size() > 78 || _props.size() > 78)
    {
        std::string_view text = _text;
        std::span<const uint8_t> props(_props);
        
        std::vector<std::string_view> lines;
        std::vector<std::span<const uint8_t>> prop_lines;
        
        while(text.size() > 78)
        {
            lines.push_back(text.substr(0, 78));
            text = text.substr(78);
        }
        lines.push_back(text);
        
        while(props.size() > 78)
        {
            prop_lines.push_back(props.subspan(0, 78));
            props = props.subspan(78);
        }
        prop_lines.push_back(props);
        
        size_t n1 = lines.size();
        size_t n2 = prop_lines.size();
        
        size_t n = std::max(n1, n2);
        for(size_t i = 0; i < n; i++)
        {
            if(i < n1 && i < n2)
            {
                InternalAddConsoleLine(lines[i], prop_lines[i]);
            }
            else if(i < n1)
            {
                InternalAddConsoleLine(lines[i], {});
            }
            else if(i < n2)
            {
                InternalAddConsoleLine("", prop_lines[i]);
            }
        }
    }
    else
    {
        InternalAddConsoleLine(_text, _props);
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
