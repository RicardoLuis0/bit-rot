#include "Command.h"
#include "Game.h"
#include "Common.h"

using namespace Game;

using enum dir_entry_type;
using enum hide_type;

std::map<dir_entry_type, std::string> dir_type_strings
{
    {FOLDER,        "FOLDER"},
    {TEXT,          " TEXT "},
    {DATA,          " BIN  "},
    {PROGRAM,       " EXE  "},
    {PROGRAM_ALIAS, "ALIAS "},
    {DRIVER,        " SYS  "},
};

void Command::Dir(const std::vector<std::string> &args)
{
    std::string folder;
    
    AddConsoleLine("");
    if(args.size() > 2)
    {
        AddConsoleLine("Too many Arguments passed to DIR");
        AddConsoleLine("");
        return;
    }
    else if(args.size() > 1)
    {
        if(!HasAccess(args[1], "DIR", &folder, nullptr, FOLDER, VISIBLE, false))
        {
            AddConsoleLine("");
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
    AddConsoleLine("");
}
