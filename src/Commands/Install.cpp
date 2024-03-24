#include "Command.h"
#include "Game.h"
#include "Common.h"

using namespace Game;

using enum dir_entry_type;
using enum hide_type;

void Command::Install(const std::vector<std::string> &args)
{
    AddConsoleLine("");
    if(args.size() > 2)
    {
        AddConsoleLine("Too many Arguments passed to INSTALL");
        AddConsoleLine("");
        return;
    }
    else if(args.size() < 2)
    {
        AddConsoleLine("Too few Arguments passed to INSTALL");
        AddConsoleLine("");
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
    AddConsoleLine("");
}
