#include "Command.h"
#include "Game.h"
#include "Common.h"
#include "SaveData.h"

using namespace Game;

using enum dir_entry_type;
using enum hide_type;

int Command::Recovery(const std::vector<std::string> &args)
{
    AddConsoleLine("");
    if(args.size() > 2)
    {
        AddConsoleLine("Too many Arguments passed to RECOVERY");
        AddConsoleLine("");
        return 0;
    }
    
    std::string folder = currentFolder;
    
    if(args.size() > 1)
    {
        if(!HasAccess(args[1], "RECOVERY", &folder, nullptr, FOLDER, VISIBLE, false))
        {
            AddConsoleLine("");
            return 0;
        }
    }
    
    auto &entries = directories[currentDrive][folder];
    
    std::vector<std::string> recovered;
    for(auto & entry : entries)
    {
        if(entry.second.hidden == DELETED || entry.second.hidden == CORRUPTED)
        {
            SaveData::PushAction(SaveData::RECOVERY, currentFolder + entry.second.name);
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
        AddConsoleLine("");
        return 1;
    }
    else
    {
        AddConsoleLine("Could not recover any files");
        AddConsoleLine("");
        return 0;
    }
}
