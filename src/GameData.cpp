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
    {{"666"}, program_help::hide({stringRand("________________________________________________________________________________________________________________________________________", '_', 819), stringRand("_________________", '_', 820)})},
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

void Game::LoadData()
{
    std::string texts = Util::ReadFile("GameData/data.json");
    JSON::Element e = JSON::Parse(texts);
    
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
                        throw std::runtime_error("Invalid directory type in data.json");//TODO improve error
                    }
                    entries.emplace(std::pair{files.first, dir_entry(arr[0].get_str(), dir_entry_types[arr[1].get_str()])});
                    break;
                case 3:
                    //type + attr
                    if(!dir_entry_types.contains(arr[1].get_str()))
                    {
                        throw std::runtime_error("Invalid directory type in data.json");//TODO improve error
                    }
                    if(!hide_types.contains(arr[2].get_str()))
                    {
                        throw std::runtime_error("Invalid directory attribute in data.json");//TODO improve error
                    }
                    entries.emplace(std::pair{files.first, dir_entry(arr[0].get_str(), dir_entry_types[arr[1].get_str()], hide_types[arr[2].get_str()])});
                    break;
                case 4:
                    //type + encrypted attr + pass
                    if(!dir_entry_types.contains(arr[1].get_str()))
                    {
                        throw std::runtime_error("Invalid directory type in data.json");//TODO improve error
                    }
                    if(arr[2].get_str() == "ENCRYPTED")
                    {
                        entries.emplace(std::pair{files.first, dir_entry(arr[0].get_str(), dir_entry_types[arr[1].get_str()], ENCRYPTED, arr[3].get_str())});
                        break;
                    }
                    [[fallthrough]];
                default:
                    throw std::runtime_error("Invalid parameter count for directory in data.json");
                }
            }
            dirs.emplace(std::pair{folder.first, std::move(entries)});
        }
        
        directories.emplace(std::pair{drive.first, std::move(dirs)});
        
    }
}
