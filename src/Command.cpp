#include "Command.h"
#include "Game.h"
#include "Common.h"

using namespace Game;

using enum dir_entry_type;
using enum hide_type;

int Unimplemented(const std::string &command_name, const std::vector<std::string> &args)
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
    return 0;
}

int Command::Decrypt(const std::vector<std::string> &args)
{
    Unimplemented("DECRYPT", args);
    return 0;
    //TODO
}

extern int currentScreen;

int Command::EndJamBuild(const std::vector<std::string> &args)
{
    currentScreen = 5;
}
