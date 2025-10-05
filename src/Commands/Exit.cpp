#include "Command.h"
#include "Game.h"

using namespace Game;

extern bool RunGame;

int Command::Exit(const std::vector<std::string> &args)
{
    if(args.size() > 1)
    {
        AddConsoleLine("Too many Arguments passed to EXIT");
        return 0;
    }
    else
    {
        RunGame = false;
        return 1;
    }
}
