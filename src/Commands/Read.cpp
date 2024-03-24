#include "Command.h"
#include "Game.h"
#include "Common.h"

using namespace Game;

using enum dir_entry_type;
using enum hide_type;

void Command::Read(const std::vector<std::string> &args)
{
    AddConsoleLine("");
    if(args.size() > 2)
    {
        AddConsoleLine("Too many Arguments passed to READ");
        AddConsoleLine("");
        return;
    }
    else if(args.size() < 2)
    {
        AddConsoleLine("Too few Arguments passed to READ");
        AddConsoleLine("");
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
    AddConsoleLine("");
}
