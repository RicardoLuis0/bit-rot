#include "Command.h"
#include "Game.h"
#include "Common.h"

using namespace Game;

using enum dir_entry_type;
using enum hide_type;

void Command::Help(const std::vector<std::string> &args)
{
    AddConsoleLine("");
    if(args.size() == 1)
    {
        std::vector<std::string> programs = Game::ListPrograms();
        AddConsoleLine("Installed Programs: (use HELP <PROGRAM> for usage info)");
        AddConsoleLine("");
        for(const std::pair<const std::vector<std::string>, program_help> &cmd : programHelp)
        {
            if(cmd.second.hidden) continue;
            
            std::vector<std::string> found;
            
            for(const std::string &f : cmd.first)
            {
                if(std::find(programs.begin(), programs.end(), f) != programs.end())
                {
                    found.push_back(f);
                }
            }
            if(found.size() == 0) continue;
            
            std::string start = " \07 "+Util::Join(found, " / ")+" - ";
            
            if(cmd.second.help[0].second.size() > 0)
            {
                AddConsoleLine(start+cmd.second.help[0].first, Util::Concat(std::vector<uint8_t>(start.size(), 0),cmd.second.help[0].second));
            }
            else
            {
                AddConsoleLine(start+cmd.second.help[0].first);
            }
        }
        AddConsoleLine("");
    }
    else if(args.size() == 2)
    {
        std::vector<std::string> programs = Game::ListPrograms();
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
                    
                    for(auto &line : cmd.second.help)
                    {
                        std::string start = "  ";
                        
                        if(line.second.size() > 0)
                        {
                            AddConsoleLine(start+line.first, Util::Concat(std::vector<uint8_t>(start.size(), 0),line.second));
                        }
                        else
                        {
                            AddConsoleLine(start+line.first);
                        }
                    }
                    
                    AddConsoleLine("");
                    AddConsoleLine("Usage:");
                    AddConsoleLine("");
                    
                    for(auto &line : cmd.second.usage)
                    {
                        std::string start = " \07 ";
                        
                        if(line.second.size() > 0)
                        {
                            AddConsoleLine(start+line.first, Util::Concat(std::vector<uint8_t>(start.size(), 0),line.second));
                        }
                        else
                        {
                            AddConsoleLine(start+line.first);
                        }
                    }
                    AddConsoleLine("");
                    
                    return;
                }
            }
            
        }
        AddConsoleLine("No HELP found for "+Util::QuoteString(args[1]));
        AddConsoleLine("");
    }
    else
    {
        AddConsoleLine("Too many Arguments passed to HELP");
        AddConsoleLine("");
    }
}
