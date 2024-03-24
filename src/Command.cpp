#include "Command.h"
#include "Game.h"
#include "Common.h"

using namespace Game;

using enum dir_entry_type;
using enum hide_type;

void Unimplemented(const std::string &command_name, const std::vector<std::string> &args)
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
}

void Command::Decrypt(const std::vector<std::string> &args)
{
    Unimplemented("DECRYPT", args);
    //TODO
}

extern int currentScreen;

void Command::EndJamBuild(const std::vector<std::string> &args)
{
    currentScreen = 5;
}
