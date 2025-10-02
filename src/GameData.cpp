#include "Game.h"
#include "Common.h"
#include "Json.h"
#include "Input.h"
#include "Renderer.h"
#include "SDL2Util.h"
#include "Command.h"

using enum dir_entry_type;
using enum hide_type;

std::string currentDrive = "C";
std::string currentFolder = "\\";

std::map<std::string, CommandProc> programs
{
    {"CD", &Command::Cd},
    {"DIR", &Command::Dir},
    {"LS", &Command::Dir},
    {"CLS", &Command::Clear},
    {"CLEAR", &Command::Clear},
    {"HELP", &Command::Help},
    {"READ", &Command::Read},
    {"CAT", &Command::Read},
    {"INSTALL", &Command::Install},
    {"UNLOCK", &Command::Unlock},
    {"RECOVERY", &Command::Recovery},
    {"EXIT", &Command::Exit},
    //{"DECRYPT", &Command::Decrypt}, // TODO
    {"666", &Command::EndJamBuild},
};

std::map<std::vector<std::string>, program_help> programHelp
{
    {{"HELP"}, {"Show Help for Commands", {"HELP"_sv, "HELP <COMMAND>"_sv}}},
    {{"CD"}, {"Change directories or show the path of the current directory (use '..' to go back to the previous directory)", "CD <DIR>"}},
    {{"DIR", "LS"}, {"List entries of the current directory", {"LS", "LS <PATH>", "DIR", "DIR <PATH>"}}},
    {{"CLS", "CLEAR"}, {"Clear screen", {"CLS"_sv, "CLEAR"_sv}}},
    {{"CAT", "READ"}, {"Read text documents", {"CAT <PATH>"_sv, "READ <PATH>"_sv}}},
    {{"INSTALL"}, {"Install a program to your \\BIN\\ directory, allowing you to use it anywhere", "INSTALL <PATH>"}},
    {{"UNLOCK"}, {"Pass it a path and a password to unlock encrypted folders/files", "UNLOCK <PATH> <PASSWORD>"}},
    {{"RECOVERY"}, {"This can recover deleted files in the current folder", "RECOVERY"}},
    {{"EXIT"}, {"Save and Exit", "EXIT"}},
    {{"666"}, program_help::hide({stringRandDyn("________________________________________________________________________________________________________________________________________", '_'), stringRandDyn("_________________", '_')})},
};

std::map<std::string, std::map<std::string, std::map<std::string, dir_entry>>> directories;

std::string JoinText(JSON::Element e)
{
    if(e.is_arr())
    {
        return Util::Join(Util::Map(e.get_arr(), &JoinText), "");
    }
    else
    {
        return e.get_str();
    }
}

std::map<std::string, std::string> textFilesCorrupted;
std::map<std::string, std::string> textFiles;
std::map<std::string, std::string> texts;

std::map<std::string, dir_entry_type> dir_entry_types
{
    {"FOLDER",FOLDER},
    {"TEXT", TEXT},
    {"DATA", DATA},
    {"PROGRAM", PROGRAM},
    {"PROGRAM_ALIAS", PROGRAM_ALIAS},
    {"DRIVER", DRIVER},
};
std::map<std::string, hide_type> hide_types
{
    {"VISIBLE", VISIBLE},
    {"CORRUPTED", CORRUPTED},
    //{"ENCRYPTED", ENCRYPTED},
    {"DELETED", DELETED},
    {"HIDDEN", HIDDEN},
    {"FORBIDDEN", FORBIDDEN},
};

constexpr const char * dataFile = "GameData/data.json";

std::vector<initTextLine> initText;
uint32_t numRecoveryTexts;

void Game::LoadData() try
{
    JSON::Element e = JSON::Parse(Util::ReadFile(dataFile));
    
    JSON::Element corruptedfiles = e["CorruptedTextFiles"];
    int corruptedseed = 123;
    for(auto &pair : corruptedfiles.get_obj())
    {
        textFilesCorrupted[pair.first] = stringRand(JoinText(pair.second), '_', corruptedseed++);
    }
    
    JSON::Element files = e["TextFiles"];
    for(auto &pair : files.get_obj())
    {
        textFiles[pair.first] = JoinText(pair.second);
    }
    
    JSON::Element game_texts = e["Texts"];
    for(auto &pair : game_texts.get_obj())
    {
        texts[pair.first] = JoinText(pair.second);
    }
    
    JSON::Element game_intro_texts = e["IntroTexts"];
    for(JSON::Element &line : game_intro_texts.get_arr())
    {
        initTextLine l;
        l.timer = line[0].get_int();
        l.text = line[1].get_str();
        l.beep = line[2].get_bool();
        l.recovery = line[3].get_bool();
        l.intro = line[4].get_bool();
        initText.push_back(std::move(l));
    }
    
    numRecoveryTexts = Util::Reduce<uint32_t>(initText,
        [](const initTextLine& line, uint32_t acc)
        {
            return (line.intro) ? acc : acc + 1;
        }
    );
    
    JSON::Element drives = e["Drives"];
    
    for(auto &drive : drives.get_obj())
    {
        std::map<std::string, std::map<std::string, dir_entry>> dirs = {};
        
        for(auto &folder : drive.second.get_obj())
        {
            std::map<std::string, dir_entry> entries;
            for(auto &files : folder.second.get_obj())
            {
                auto &arr = files.second.get_arr();
                switch(arr.size())
                {
                case 2:
                    //type
                    if(!dir_entry_types.contains(arr[1].get_str()))
                    {
                        throw FatalError("Invalid directory type in data.json");//TODO improve error
                    }
                    entries.emplace(std::pair{files.first, dir_entry(arr[0].get_str(), dir_entry_types[arr[1].get_str()])});
                    break;
                case 3:
                    //type + attr
                    if(!dir_entry_types.contains(arr[1].get_str()))
                    {
                        throw FatalError("Invalid directory type in data.json");//TODO improve error
                    }
                    if(!hide_types.contains(arr[2].get_str()))
                    {
                        throw FatalError("Invalid directory attribute in data.json");//TODO improve error
                    }
                    entries.emplace(std::pair{files.first, dir_entry(arr[0].get_str(), dir_entry_types[arr[1].get_str()], hide_types[arr[2].get_str()])});
                    break;
                case 4:
                    //type + encrypted attr + pass
                    if(!dir_entry_types.contains(arr[1].get_str()))
                    {
                        throw FatalError("Invalid directory type in data.json");//TODO improve error
                    }
                    if(arr[2].get_str() == "ENCRYPTED")
                    {
                        entries.emplace(std::pair{files.first, dir_entry(arr[0].get_str(), dir_entry_types[arr[1].get_str()], ENCRYPTED, arr[3].get_str())});
                        break;
                    }
                    [[fallthrough]];
                default:
                    throw FatalError("Invalid parameter count for directory in data.json");
                }
            }
            dirs.emplace(std::pair{folder.first, std::move(entries)});
        }
        
        directories.emplace(std::pair{drive.first, std::move(dirs)});
        
    }
}
catch(JSON::JSON_Exception &e)
{
    throw FatalError("Malformed JSON in "+Util::QuoteString(dataFile)+": "+e.msg_top);
}
