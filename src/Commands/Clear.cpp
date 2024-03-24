#include "Command.h"
#include "Game.h"
#include "Common.h"

using namespace Game;

using enum dir_entry_type;
using enum hide_type;

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
