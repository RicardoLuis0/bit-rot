#include "Common.h"
#include "Game.h"
#include "Input.h"
#include "SaveData.h"
#include "SDL2Util.h"

extern int currentScreen;

using enum dir_entry_type;
using enum hide_type;

bool lastWasTab = false;

std::vector<dir_entry> ListFolders(std::string path)
{
    std::string folder;
    if(Game::HasAccess(path, "", &folder, nullptr, FOLDER, VISIBLE, false, true))
    {
        return Game::ListDir(folder);
    }
    else
    {
        return {};
    }
}


static const std::string &DirName(const dir_entry& e)
{
    return e.name;
}

Game::ShellContext Game::rootShellContext;

void Game::Responder(SDL_Event *e)
{
    switch(e->type)
    {
    case SDL_KEYDOWN:
        if(e->key != SDLK_TAB)
        {
            lastWasTab = false;
        }
        
        if(e->key == SDLK_ESCAPE)
        {
            currentScreen = 2; // pause
        }
        else if(e->key == SDLK_UP)
        {
            bool isMax = tempCommandPos == tempCommand.size();
            if(historyPos == -1)
            {
                tempCommandPreHistory = tempCommand;
            }
            historyPos++;
            tempCommand = commandHistory[historyPos];
            tempCommandPos = isMax ? tempCommand.size() : std::min(tempCommandPos, tempCommand.size());
        }
        else if(e->key == SDLK_DOWN)
        {
            bool isMax = tempCommandPos == tempCommand.size();
            if(historyPos > 0)
            {
                historyPos--;
                tempCommand = commandHistory[historyPos];
                tempCommandPos = isMax ? tempCommand.size() : std::min(tempCommandPos, tempCommand.size());
            }
            else if(historyPos == 0)
            {
                historyPos--;
                tempCommand = tempCommandPreHistory;
                tempCommandPos = isMax ? tempCommand.size() : std::min(tempCommandPos, tempCommand.size());
            }
            else
            {
                tempCommandPos = tempCommand.size();
            }
        }
        else if(e->key == SDLK_RIGHT)
        {
            if(tempCommandPos < tempCommand.size())
            {
                if(Input::CtrlPressed())
                {
                    size_t p = tempCommand.find_first_of(' ', tempCommandPos + 1);
                    if(p != std::string::npos)
                    {
                        tempCommandPos = p + 1;
                    }
                    else
                    {
                        tempCommandPos = tempCommand.size();
                    }
                }
                else
                {
                    tempCommandPos++;
                }
            }
        }
        else if(e->key == SDLK_LEFT)
        {
            if(tempCommandPos > 0)
            {
                if(Input::CtrlPressed())
                {
                    size_t p = tempCommand.find_last_of(' ', tempCommandPos - ((tempCommand[tempCommandPos - 1] == ' ' && (tempCommandPos - 2) > 0) ? 2 : 1));
                    if(p != std::string::npos)
                    {
                        tempCommandPos = p;
                    }
                    else
                    {
                        tempCommandPos = 0;
                    }
                }
                else
                {
                    tempCommandPos--;
                }
            }
        }
        else if(e->key == SDLK_BACKSPACE && tempCommandPos > 0 && tempCommand.size() > 0)
        {
            tempCommandPos--;
            if(tempCommandViewOffset > 0) tempCommandViewOffset--;
            tempCommand.erase(tempCommandPos, 1);
            historyPos = -1;
            tempCommandPreHistory = "";
        }
        else if(e->key == SDLK_RETURN)
        {
            std::string command = tempCommand;
            tempCommandPos = 0;
            tempCommand = "";
            
            if(CommandLineDrawPath)
            {
                Game::AddConsoleLine(currentDrive + ":" + ((currentFolder.size() > 1) ? currentFolder.substr(0, currentFolder.size() - 1) : currentFolder) + ">" + command);
            }
            else
            {
                Game::AddConsoleLine(">" + command);
            }
            
            commandHistory.insert(commandHistory.begin(), command);
            SaveData::PushHistory(command);
            
            RunCommand(command, rootShellContext);
            historyPos = -1;
            tempCommandPreHistory = "";
        }
        else if(e->key == SDLK_TAB)
        {
            std::vector<Util::SplitPoint> args = Util::SplitStringEx(tempCommand, ' ', true, true);
            
            const Util::SplitPoint * currentArg = nullptr;
            const Util::SplitPoint * newArg = &*args.end();
            
            for(auto arg = args.begin(); arg < args.end(); arg++)
            {
                if(arg->offset > tempCommandPos) break;
                currentArg = &*arg;
            }
            
            if(currentArg != nullptr && currentArg == args.data() + (args.size() - 1) && tempCommandPos > (currentArg->offset + currentArg->orig_len))
            { // past the end treat as "new" arg
                currentArg = newArg;
            }
            
            if(currentArg == nullptr || currentArg == newArg || currentArg->to_view().empty())
            {
                if(lastWasTab)
                {
                    std::string alts = "";
                    if(currentArg == nullptr || currentArg == args.data() || (currentArg == newArg && args.size() == 1 && Util::StrToUpper(args[0].to_view()) == "HELP"))
                    {
                        alts = Util::Join(ListExecutablePrograms(), " ");
                    }
                    else
                    {
                        alts = Util::Join(Util::Map(ListFolders(currentFolder), DirName), " ");
                    }
                    lastWasTab = false;
                    
                    if(CommandLineDrawPath)
                    {
                        Game::AddConsoleLine(currentDrive + ":" + ((currentFolder.size() > 1) ? currentFolder.substr(0, currentFolder.size() - 1) : currentFolder) + ">" + tempCommand);
                    }
                    else
                    {
                        Game::AddConsoleLine(">" + tempCommand);
                    }
                    
                    Util::ForEach(Util::SplitLines((Input::ShiftPressed() || (e->key.keysym.mod & KMOD_CAPS)) ? alts : Util::StrToLower(alts), 78), (void(*)(std::string_view))Game::AddConsoleLine);
                }
                else
                {
                    lastWasTab = true;
                }
                break;
            }
            else if(currentArg == args.data() || (currentArg == args.data() + 1 && Util::StrToUpper(args[0].to_view()) == "HELP"))
            { // program name, TODO dehardcode it from help (leave type handling per command) if any new commands take non-paths
                std::string arg = Util::StrToUpper(currentArg->to_view());
                std::vector<std::string> completionAlternatives =
                                                Util::Filter(ListExecutablePrograms(),
                                                        std::bind((bool(std::string::*)(std::string_view)const noexcept)&std::string::starts_with, std::placeholders::_1, std::string_view(arg)));
                if(completionAlternatives.size() > 1)
                {
                    if(lastWasTab)
                    {
                        std::string alts = Util::Join(completionAlternatives, " ");
                        lastWasTab = false;
                        
                        if(CommandLineDrawPath)
                        {
                            Game::AddConsoleLine(currentDrive + ":" + ((currentFolder.size() > 1) ? currentFolder.substr(0, currentFolder.size() - 1) : currentFolder) + ">" + tempCommand);
                        }
                        else
                        {
                            Game::AddConsoleLine(">" + tempCommand);
                        }
                        
                        Util::ForEach(Util::SplitLines((Input::ShiftPressed() || (e->key.keysym.mod & KMOD_CAPS)) ? alts : Util::StrToLower(alts), 78), (void(*)(std::string_view))Game::AddConsoleLine);
                    }
                    else
                    {
                        lastWasTab = true;
                    }
                }
                else if(completionAlternatives.size() == 1)
                {
                    if(completionAlternatives[0] != arg)
                    {
                        tempCommand.replace(currentArg->offset, currentArg->orig_len, (Input::ShiftPressed() || (e->key.keysym.mod & KMOD_CAPS)) ? completionAlternatives[0] : Util::StrToLower(completionAlternatives[0]));
                        tempCommandPos = currentArg->offset + completionAlternatives[0].size();
                        if(currentArg == args.data() + (args.size() - 1))
                        {
                            tempCommand += " ";
                            tempCommandPos++;
                        }
                    }
                    else if(tempCommandPos == currentArg->offset + currentArg->orig_len && currentArg == args.data() + (args.size() - 1))
                    {
                        tempCommand += " ";
                        tempCommandPos++;
                    }
                }
            }
            else
            { // path
                std::vector<std::string> path_vec = Util::MapInplace(Util::SplitString(currentArg->to_view(), "\\/", false, false), Util::StrToUpper);
                std::string path = "";
                std::string partial = "";
                if(path_vec.size() == 0 || ((path_vec.size() == 1 && path_vec[0].empty()) && !(currentArg->to_view().length() == 1 && (currentArg->to_view()[0] == '\\' || currentArg->to_view()[0] == '/'))))
                {
                    break;
                }
                
                if(currentArg->to_view().back() == '\\' || currentArg->to_view().back() == '/')
                {
                    path += Util::Join(path_vec, "\\");
                }
                else if(path_vec.size() > 0)
                {
                    if(path_vec.size() > 1)
                    {
                        path += Util::Join(std::span(path_vec.begin(), path_vec.end() -1), "\\");
                    }
                    partial = path_vec.back();
                }
                
                if((currentArg->to_view().front() == '\\' || currentArg->to_view().front() == '/') && (path.size() == 0 || path[0] != '\\'))
                {
                    path = "\\" + path;
                }
                
                std::vector<dir_entry> completionAlternatives =
                (
                    (partial == "") ?
                        ListFolders(path) :
                        Util::Filter(ListFolders(path), [p = std::string_view(partial)](dir_entry &e){return e.name.starts_with(p);})
                );
                
                if(completionAlternatives.size() > 1)
                {
                    if(lastWasTab)
                    {
                        std::string alts = Util::Join(Util::Map(completionAlternatives, DirName), " ");
                        lastWasTab = false;
                        
                        if(CommandLineDrawPath)
                        {
                            Game::AddConsoleLine(currentDrive + ":" + ((currentFolder.size() > 1) ? currentFolder.substr(0, currentFolder.size() - 1) : currentFolder) + ">" + tempCommand);
                        }
                        else
                        {
                            Game::AddConsoleLine(">" + tempCommand);
                        }
                        
                        Util::ForEach(Util::SplitLines((Input::ShiftPressed() || (e->key.keysym.mod & KMOD_CAPS)) ? alts : Util::StrToLower(alts), 78), (void(*)(std::string_view))Game::AddConsoleLine);
                    }
                    else
                    {
                        lastWasTab = true;
                    }
                }
                else if(completionAlternatives.size() == 1)
                {
                    std::string alt = ((path.empty() || path.back() == '\\') ? path : path + "\\") + completionAlternatives[0].name;
                    
                    if(completionAlternatives[0].name != partial)
                    {
                        tempCommand.replace(currentArg->offset, currentArg->orig_len, (Input::ShiftPressed() || (e->key.keysym.mod & KMOD_CAPS)) ? alt : Util::StrToLower(alt));
                        tempCommandPos = currentArg->offset + alt.size();
                    }
                    else if(tempCommandPos == currentArg->offset + currentArg->orig_len && currentArg == args.data() + (args.size() - 1) && completionAlternatives[0].type == FOLDER)
                    {
                        tempCommand += "\\";
                        tempCommandPos++;
                    }
                }
            }
        }
        /*
        else if(e->key.keysym.sym >= ' ' && e->key.keysym.sym <= '~')
        {
            char c = e->key.keysym.sym;
            if(Input::ShiftPressed() || (e->key.keysym.mod & KMOD_CAPS))
            {
                c = Util::CharToUpper(c);
            }
            tempCommand.insert(tempCommandPos, std::string(1, c));
            tempCommandPos++;
            historyPos = -1;
            tempCommandPreHistory = "";
        }
        */
        break;
    case SDL_TEXTINPUT:
        //TODO skip all unicode
        for(char * cc = e->text.text; *cc; cc++)
        {
            char c = *cc;
            tempCommand.insert(tempCommandPos, std::string(1, c));
            tempCommandPos++;
            historyPos = -1;
            tempCommandPreHistory = "";
        }
        break;
    }
}

int LastCmd = 0;

using ShellBuiltin = int(*)(Game::ShellContext &ctx, std::vector<std::string> &args, const std::string &rawargs);

std::map<std::string, ShellBuiltin> shellBuiltIns
{
    {"TRUE", [](auto,auto,auto){return 1;}},
    {"FALSE", [](auto,auto,auto){return 0;}},
    {"!", [](Game::ShellContext &ctx,auto, const std::string &rawargs) -> int
    {
        return !RunCommand(rawargs, ctx);
    }},
    {"ECHO", [](auto,std::vector<std::string> &args,auto)
    {
        args.erase(std::begin(args));
        Game::AddConsoleLine("");
        Game::AddConsoleLine(Util::Join(args, " "));
        Game::AddConsoleLine("");
        return 1;
    }},
};

extern bool RunGame;

static int RunStatementList(const std::vector<std::string> &stmts, Game::ShellContext &ctx)
{
    int last = 0;
    
    for(const std::string &cmd : stmts)
    {
        std::string cmd_trim = Util::TrimFrontBack(cmd, " \t"); // remove leading/trailing whitespace
        if(cmd_trim.length() > 0)
        {
            last = Game::RunCommand(cmd_trim, ctx);
            if(currentScreen != 4 || !RunGame) break;
        }
    }
    
    return last;
}

struct scriptStatement
{
    std::string type;//if/while/for/list
    size_t offset;
    size_t len;
    //TODO
};

int Game::RunCommand(const std::string &command, ShellContext &ctx)
{
    //TODO parse if/then/fi, while/do/done, for/do/done
    std::vector<std::string> commandQueue = Util::SplitString(command, ";\n", true, true, true, {{"(",")"}}); // split on newline and ';', but keep everything inside () unsplit for subshells
    
    if(commandQueue.size() > 1)
    { // process script
        return RunStatementList(commandQueue, ctx);
    }
    else
    {
        std::vector<std::string> shellops = {"&&", "||", "(", ")"}; //, "$(", "|", "[", "]"};
        
        std::vector<std::variant<Util::SplitOp, Util::SplitPoint>> ops = Util::SplitStringOp(command, shellops);
        
        if(ops.size() == 0)
        {
            Game::AddConsoleLine("");
            Game::AddConsoleLine("Could not find program ''");
            Game::AddConsoleLine("");
            return 0;
        }
        else if(!(ops.size() == 1 && std::holds_alternative<Util::SplitPoint>(ops[0])))
        {
            int result = 0;
            std::string op = "";
            size_t n = ops.size();
            for(size_t i = 0; i < n; i++)
            {
                if(std::holds_alternative<Util::SplitPoint>(ops[i]))
                {
                    if(op == "||" && result) continue; // short circuit
                    else if(op == "&&" && !result) continue; // short circuit
                    else
                    {
                        int r = RunCommand(std::get<Util::SplitPoint>(ops[i]).str, ctx);
                        if(op == "||")
                        {
                            result = result || r;
                        }
                        else if(op == "&&")
                        {
                            result = result && r;
                        }
                        else
                        {
                            result = r;
                        }
                        op = "";
                    }
                }
                else
                {
                    auto o = std::get<Util::SplitOp>(ops[i]);
                    
                    if(o.op == ")")
                    {
                        Game::AddConsoleLine("");
                        Game::AddConsoleLine("Mismatched ')'");
                        Game::AddConsoleLine("");
                        
                        return 0;
                    }
                    else if(o.op == "(")
                    {
                        //TODO implement subshell
                        
                        int depth = 1;
                        int r = 0;
                        
                        size_t begin = 0;
                        size_t end = 0;
                        
                        bool has_begin = false;
                        
                        i++;
                        while(depth > 0 && i < n)
                        {
                            if(std::holds_alternative<Util::SplitPoint>(ops[i]))
                            {
                                auto &p = std::get<Util::SplitPoint>(ops[i]);
                                if(!has_begin)
                                {
                                    begin = p.offset;
                                    has_begin = true;
                                }
                                end = p.offset + p.orig_len;
                            }
                            else
                            {
                                auto o2 = std::get<Util::SplitOp>(ops[i]);
                                if(o2.op == "(")
                                {
                                    depth++;
                                }
                                else if(o2.op == ")")
                                {
                                    depth--;
                                }
                                
                                if(depth > 0)
                                {
                                    end = o2.offset + o2.op.length();
                                }
                            }
                            i++;
                        }
                        i--;
                        
                        
                        if(op == "||" && result)
                        {
                            // short circuit
                        }
                        else if(op == "&&" && !result)
                        {
                            // short circuit
                        }
                        else
                        {
                        
                            if(!has_begin)
                            {
                                Game::AddConsoleLine("");
                                Game::AddConsoleLine("Could not find program ''");
                                Game::AddConsoleLine("");
                            }
                            else
                            {
                                ShellContext subctx = ctx;
                                
                                std::string tempDrive = currentDrive;
                                std::string tempFolder = currentFolder;
                                
                                r = RunCommand(command.substr(begin, end-begin), subctx);
                                
                                currentDrive = tempDrive;
                                currentFolder = tempFolder;
                                SaveData::SetFolder(currentFolder);
                            }
                            
                            if(op == "||")
                            {
                                result = result || r;
                            }
                            else if(op == "&&")
                            {
                                result = result && r;
                            }
                            else
                            {
                                result = r;
                            }
                            
                            op = "";
                        }
                        
                        return 0;
                    }
                    else
                    {
                        if(op != "")
                        {
                            Game::AddConsoleLine("");
                            Game::AddConsoleLine("Unexpected '"+o.op+"' after '"+op+"'");
                            Game::AddConsoleLine("");
                            return 0;
                        }
                        else
                        {
                            op = o.op;
                        }
                    }
                }
            }
            
            return result;
        }
        else
        {
            std::vector<std::string> programsList = Game::ListExecutablePrograms();
            
            std::vector<Util::SplitPoint> argsEx = Util::SplitStringEx(command, ' ', true, true, true);
            
            bool firstIsAssign = false;
            bool firstRead = false;
            
            std::vector<std::string> args = Util::Map(argsEx, [&ctx,&firstIsAssign,&firstRead](auto &arg)
            {
                auto split = Util::SplitStringQuotes(arg.str);
                
                std::string str = "";
                
                size_t index = 0;
                
                for(auto &quote : split)
                {
                    if(quote.c == 0)
                    {
                        if(quote.str[0] == '$')
                        { // variable
                            std::string varname = quote.str.substr(1uz, quote.str.find('=') - 1uz); // '=' is not allowed in var names
                            
                            if(varname == "?")
                            {
                                str += std::to_string(LastCmd);
                            }
                            else
                            {
                                const auto &var = ctx.variables.find(varname);
                                if(var != ctx.variables.end())
                                {
                                    str += var->second;
                                }
                            }
                        }
                        else
                        {
                            if(!firstRead && quote.str.find('=') != std::string::npos)
                            {
                                firstIsAssign = true;
                            }
                            str += quote.str;
                        }
                    }
                    else if(quote.c == '\'')
                    {
                        str += quote.str;
                    }
                    else if(quote.c == '"')
                    {
                        size_t pos = 0;
                        do
                        {
                            size_t pos_start = quote.str.find('$', pos);
                            
                            if(pos_start == std::string::npos)
                            {
                                str += quote.str.substr(pos);
                                break; // no more vars
                            }
                            
                            if(quote.was_escaped[pos_start]) // is \$ not $, find next
                            {
                               pos = pos_start + 1; 
                            }
                            else
                            {
                                str += quote.str.substr(pos, pos_start - pos);
                                
                                size_t s = pos_start + 1;
                                size_t pos_end = quote.str.find_first_of(std::string(" \n\t=\0", 5), s);
                                
                                std::string varname = quote.str.substr(s, pos_end - s);
                                
                                if(varname == "?")
                                {
                                    str += std::to_string(LastCmd);
                                }
                                else
                                {
                                    const auto &var = ctx.variables.find(varname);
                                    if(var != ctx.variables.end())
                                    {
                                        str += var->second;
                                    }
                                }
                                
                                pos = pos_end;
                            }
                        }
                        while(pos < std::string::npos);
                    }
                    index++;
                }
                
                firstRead = true;
                return str;
            });
            
            //unlike bash, single quotes aren't literal, they also process escape chars like double quotes, they just don't process variables
            
            if(firstIsAssign)
            {
                size_t assignpos = args[0].find('=');
                std::string varname = args[0].substr(0, assignpos);
                
                ctx.variables[varname] = args[0].substr(assignpos + 1);
                
                if(&ctx == &rootShellContext)
                {
                    SaveData::SetConsoleVars(rootShellContext.variables);
                }
                
                if(args.size() > 1)
                {
                    Game::AddConsoleLine("");
                    Game::AddConsoleLine("Junk at end of assign, expected nothing but got '"+std::string(1, command[argsEx[2].offset])+"'");
                    Game::AddConsoleLine("");
                    return 0;
                }
                return 1;
            }
            else if(args.size() > 0)
            {
                std::string cmd = Util::StrToUpper(args[0]);
                
                if(std::find(programsList.begin(), programsList.end(), cmd) != programsList.end())
                {
                    return LastCmd = programs[cmd](args);
                }
                else if(shellBuiltIns.contains(cmd))
                {
                    return LastCmd = shellBuiltIns[cmd](ctx, args, args.size() > 1 ? command.substr(argsEx[1].offset, (argsEx.back().offset + argsEx.back().orig_len) - argsEx[1].offset) : "");
                }
                else
                {
                    Game::AddConsoleLine("");
                    Game::AddConsoleLine("Could not find program "+Util::QuoteString(cmd));
                    Game::AddConsoleLine("");
                    return 0;
                }
            }
        }
    }
    return 0;
}
