#include "Command.h"
#include "Game.h"
#include "Common.h"

using namespace Game;

extern int currentScreen;
using enum dir_entry_type;
using enum hide_type;

void Unimplemented(const std::string &command_name, const std::vector<std::string> &args)
{
    AddConsoleLine("");
    AddConsoleLine(command_name+" Command Unimplemented, args:");
    size_t count = 0;
    for(const std::string &arg : args)
    {
        AddConsoleLine("    " + std::to_string(count) + ": " + Util::QuoteString(arg, '\'', false));
        count++;
    }
    AddConsoleLine("");
}

void Command::Cd(const std::vector<std::string> &args)
{
    if(args.size() > 2)
    {
        AddConsoleLine("Too many Arguments passed to CD");
        return;
    }
    else if(args.size() > 1)
    {
        if(!HasAccess(args[1], "CD", &currentFolder, nullptr, FOLDER, VISIBLE, false))
        {
            return;
        }
    }
    AddConsoleLine("Current Directory: " + currentDrive + ":" + currentFolder);
}

std::map<dir_entry_type, std::string> dir_type_strings
{
    {FOLDER,    "FOLDER"},
    {TEXT,      " TEXT "},
    {DATA,      " BIN  "},
    {PROGRAM,   " EXE  "},
    {DRIVER,    " SYS  "},
};

void Command::Dir(const std::vector<std::string> &args)
{
    std::string folder;
    
    if(args.size() > 2)
    {
        AddConsoleLine("Too many Arguments passed to DIR");
        return;
    }
    else if(args.size() > 1)
    {
        if(!HasAccess(args[1], "DIR", &folder, nullptr, FOLDER, VISIBLE, false))
        {
            return;
        }
    }
    else
    {
        folder = currentFolder;
    }
    
    auto entries = directories[currentDrive].find(folder);
    if(entries == directories[currentDrive].end()) return; // SHOULDN'T HAPPEN
    
    auto list = Util::MapValues(entries->second);
    
    std::sort(list.begin(), list.end(), [](auto &a, auto &b)
    {
        return (a.type == b.type) ? a.name < b.name : a.type < b.type;
    });
    
    uint8_t allowed = uint8_t(VISIBLE) | uint8_t(CORRUPTED) | uint8_t(ENCRYPTED) | uint8_t(FORBIDDEN);
    
    size_t max = list.size() == 0 ? 0 : Util::MaxAll(Util::Map(list, [allowed](dir_entry& e){return (uint8_t(e.hidden) & allowed) ? e.name.size() : 0;}));
    
    if(max == 0)
    {
        AddConsoleLine("Directory of " + currentDrive + ":" + currentFolder);
        AddConsoleLine("");
        AddConsoleLine("EMPTY");
    }
    else
    {
        AddConsoleLine("Directory of " + currentDrive + ":" + currentFolder);
        AddConsoleLine("");
        
        for(auto &e : list)
        {
            if(uint8_t(e.hidden) & allowed)
            {
                auto pad = std::string(max - e.name.size(), ' ');
                AddConsoleLine(e.name + pad + " - " + dir_type_strings[e.type]);
            }
        }
    }
    //TODO
}

void Command::Clear(const std::vector<std::string> &args)
{
    if(args.size() > 1)
    {
        AddConsoleLine("Too many Arguments passed to CLEAR");
    }
    else
    {
        ClearConsole();
    }
}

std::vector<std::string> helpLines
{
    "Default Commands:",
    " * CD - Change directories or show the path of the current directory (use '..' to go back to the previous directory)",
    " * DIR / LS - List entries of the current directory",
    " * CLS / CLEAR - Clear screen",
    " * CAT / READ - Read text documents",
    " * INSTALL - Install a program to your \\BIN\\ directory, allowing you to use it anywhere",
    "Other Commands:",
    " * UNLOCK - Pass it a path and a password to unlock encrypted folders/files",
    " * RECOVERY - This can recover deleted files",
    
};

/*
std::map<std::string, std::vector<std::string>> programHelp
{
    //TODO
};
*/

void Command::Help(const std::vector<std::string> &args)
{
    
    if(args.size() == 1)
    {
        AddConsoleLines(helpLines);
        //DEFAULT HELP
    }
    else
    {
        AddConsoleLine("Too many Arguments passed to HELP");
    }
}

void Command::Read(const std::vector<std::string> &args)
{
    if(args.size() > 2)
    {
        AddConsoleLine("Too many Arguments passed to READ");
        return;
    }
    else if(args.size() < 2)
    {
        AddConsoleLine("Too few Arguments passed to READ");
        return;
    }
    
    std::string filepath;
    dir_entry * entry = nullptr;
    if(HasAccess(args[1], "READ", &filepath, &entry, TEXT, hide_type(uint8_t(VISIBLE) | uint8_t(CORRUPTED)), false) && entry)
    {
        if(entry->hidden == CORRUPTED)
        {
            AddConsoleLine(textFilesCorrupted[filepath]);
        }
        else
        {
            AddConsoleLine(textFiles[filepath]);
        }
    }
}

void Command::Recovery(const std::vector<std::string> &args)
{
    if(args.size() > 1)
    {
        AddConsoleLine("Too many Arguments passed to RECOVERY");
        return;
    }
    
    auto &entries = directories[currentDrive][currentFolder];
    
    std::vector<std::string> recovered;
    for(auto & entry : entries)
    {
        if(entry.second.hidden == DELETED || entry.second.hidden == CORRUPTED)
        {
            recovered.push_back(entry.second.name);
            entry.second.hidden = VISIBLE;
        }
    }
    
    if(recovered.size() > 0)
    {
        if(recovered.size() > 2)
        {
            AddConsoleLine("Recovered "+std::to_string(recovered.size())+" files: "+Util::JoinOr(recovered,", ", ", and"));
        }
        else if(recovered.size() == 2)
        {
            AddConsoleLine("Recovered 2 files: "+recovered[0]+" and "+recovered[1]);
        }
        else
        {
            AddConsoleLine("Recovered 1 file: "+recovered[0]);
        }
    }
    else
    {
        AddConsoleLine("Could not recover any files");
    }
}

void Command::Decrypt(const std::vector<std::string> &args)
{
    Unimplemented("DECRYPT", args);
    //TODO
}

void Command::Install(const std::vector<std::string> &args)
{
    if(args.size() > 2)
    {
        AddConsoleLine("Too many Arguments passed to INSTALL");
        return;
    }
    else if(args.size() < 2)
    {
        AddConsoleLine("Too few Arguments passed to INSTALL");
        return;
    }
    
    std::string filepath;
    dir_entry * entry = nullptr;
    if(HasAccess(args[1], "INSTALL", &filepath, &entry, ENTRY_ANY, hide_type(uint8_t(VISIBLE) | uint8_t(CORRUPTED) | uint8_t(ENCRYPTED)), false) && entry)
    {
        if(entry->type == PROGRAM)
        {
            auto &bin = directories["C"]["\\BIN\\"];
            auto bentry = bin.find(entry->name);
            if(bentry != bin.end())
            {
                AddConsoleLine(Util::QuoteString(entry->name, '\'', false)+" is already Installed");
            }
            else
            {
                bin.insert({entry->name, {entry->name, PROGRAM}});
                AddConsoleLine(Util::QuoteString(entry->name, '\'', false)+" Successfully Installed");
            }
        }
        else
        {
            AddConsoleLine(Util::QuoteString(entry->name, '\'', false)+" is not a Program");
        }
    }
}

void Command::Unlock(const std::vector<std::string> &args)
{
    if(args.size() > 3)
    {
        AddConsoleLine("Too many Arguments passed to UNLOCK");
        return;
    }
    else if(args.size() < 2)
    {
        AddConsoleLine("Too few Arguments passed to UNLOCK");
        return;
    }
    
    std::string filepath;
    dir_entry * entry = nullptr;
    if(HasAccess(args[1], "UNLOCK", &filepath, &entry, ENTRY_ANY, hide_type(uint8_t(VISIBLE) | uint8_t(CORRUPTED) | uint8_t(ENCRYPTED)), false) && entry)
    {
        if(entry->hidden != ENCRYPTED)
        {
            AddConsoleLine(Util::QuoteString(entry->name, '\'', false)+" is not Encrypted");
        }
        else if(entry->password != args[2])
        {
            AddConsoleLine("Wrong password");
        }
        else
        {
            AddConsoleLine(Util::QuoteString(entry->name, '\'', false) + " Unlocked Successfully");
            entry->hidden = VISIBLE;
        }
    }
}

void Command::EndJamBuild(const std::vector<std::string> &args)
{
    currentScreen = 5;
}
