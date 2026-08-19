#include "Command.h"
#include "Game.h"
#include "Common.h"

using namespace Game;

using enum dir_entry_type;
using enum hide_type;

void Command::PrintUsage(const std::string &cmd_name)
{
    std::vector<std::string> programs = Game::ListExecutablePrograms();
    std::string command = Util::StrToUpper(cmd_name);
    if(std::find(programs.begin(), programs.end(), command) != programs.end())
    {
        for(const std::pair<const std::vector<std::string>, program_help> &cmd : programHelp)
        {
            if(std::find(cmd.first.begin(), cmd.first.end(), command) != cmd.first.end())
            {
                AddConsoleLine("Usage:");
                AddConsoleLine("");
                for(auto &line : cmd.second.usage)
                {
                    std::string start = " \07 ";
                    
                    AddConsoleLine(start+line);
                }
            }
        }
    }
}

int Command::Help(const std::vector<std::string> &args)
{
    std::string cmdName = Util::StrToUpper(args[0]);
    AddConsoleLine("");
    if(args.size() == 1)
    {
        std::vector<std::string> programs = Game::ListExecutablePrograms();
        AddConsoleLine("Installed Programs: (use "+cmdName+" <PROGRAM> for usage info)");
        AddConsoleLine("");
        for(const std::pair<const std::vector<std::string>, program_help> &cmd : programHelp)
        {
            //if(cmd.second.hidden) continue;
            
            std::vector<std::string> found;
            
            for(const std::string &f : cmd.first)
            {
                if(std::find(programs.begin(), programs.end(), f) != programs.end())
                {
                    found.push_back(f);
                }
            }
            if(found.size() == 0) continue;
            
            std::string start = " \07 "+Util::Join(found, " / ");
            
            if(cmd.second.hidden)
            {
                AddConsoleLine(start);
            }
            else
            {
                AddConsoleLine(start+" - "+cmd.second.help);
            }
        }
        AddConsoleLine("");
        return 1;
    }
    else if(args.size() == 2)
    {
        std::vector<std::string> programs = Game::ListExecutablePrograms();
        std::string command = Util::StrToUpper(args[1]);
        
        if(std::find(programs.begin(), programs.end(), command) != programs.end())
        {
            for(const std::pair<const std::vector<std::string>, program_help> &cmd : programHelp)
            {
                if(std::find(cmd.first.begin(), cmd.first.end(), command) != cmd.first.end())
                {
                    std::vector<std::string> found;
                    
                    for(const std::string &f : cmd.first)
                    {
                        if(std::find(programs.begin(), programs.end(), f) != programs.end())
                        {
                            found.push_back(f);
                        }
                    }
                    
                    AddConsoleLine("Help for "+Util::Join(found, " / ")+":");
                    AddConsoleLine("");
                    
                    AddConsoleLine("  "+cmd.second.help);
                    
                    AddConsoleLine("");
                    AddConsoleLine("Usage:");
                    AddConsoleLine("");
                    
                    for(auto &line : cmd.second.usage)
                    {
                        std::string start = " \07 ";
                        
                        AddConsoleLine(start+line);
                    }
                    AddConsoleLine("");
                    
                    return 1;
                }
            }
        }
        AddConsoleLine("No "+cmdName+" found for "+Util::QuoteString(args[1]));
        PrintUsage("HELP");
        AddConsoleLine("");
        return 0;
    }
    else
    {
        AddConsoleLine("Too many Arguments passed to "+cmdName);
        PrintUsage("HELP");
        AddConsoleLine("");
        return 0;
    }
}
