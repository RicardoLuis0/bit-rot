#include "Command.h"
#include "Game.h"
#include "Common.h"

using namespace Game;

using enum dir_entry_type;
using enum hide_type;

void Command::Recovery(const std::vector<std::string> &args)
{
    AddConsoleLine("");
    if(args.size() > 1)
    {
        AddConsoleLine("Too many Arguments passed to RECOVERY");
        AddConsoleLine("");
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
    AddConsoleLine("");
}
