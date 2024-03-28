#include "Command.h"
#include "Game.h"
#include "Common.h"
#include "SaveData.h"

using namespace Game;

using enum dir_entry_type;
using enum hide_type;

void Command::Cd(const std::vector<std::string> &args)
{
    AddConsoleLine("");
    if(args.size() > 2)
    {
        AddConsoleLine("Too many Arguments passed to CD");
        AddConsoleLine("");
        return;
    }
    else if(args.size() > 1)
    {
        if(!HasAccess(args[1], "CD", &currentFolder, nullptr, FOLDER, VISIBLE, false))
        {
            AddConsoleLine("");
            return;
        }
        SaveData::SetFolder(currentFolder);
    }
    AddConsoleLine("Current Directory: " + currentDrive + ":" + currentFolder);
    AddConsoleLine("");
}
