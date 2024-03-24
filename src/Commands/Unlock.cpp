#include "Command.h"
#include "Game.h"
#include "Common.h"

using namespace Game;

using enum dir_entry_type;
using enum hide_type;

void Command::Unlock(const std::vector<std::string> &args)
{
    AddConsoleLine("");
    if(args.size() > 3)
    {
        AddConsoleLine("Too many Arguments passed to UNLOCK");
        AddConsoleLine("");
        return;
    }
    else if(args.size() < 2)
    {
        AddConsoleLine("Too few Arguments passed to UNLOCK");
        AddConsoleLine("");
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
    AddConsoleLine("");
}
