#include "Game.h"
#include "Common.h"
#include "Input.h"
#include "Renderer.h"
#include "SDL2Util.h"
#include "Command.h"

using enum dir_entry_type;
using enum hide_type;

std::string currentDrive = "C";
std::string currentFolder = "\\";

//names

#define YOU "Sam"
#define SISTER "Abigail"

#define FATHER "Blair"
#define MOTHER "Noelle"

#define UNCLE "Kevin"
#define UNCLE_NICKNAME "Kev"

std::map<std::string, std::string> textFilesCorrupted
{
                              //"I think there's something wrong with this computer, "
    {"\\HOME\\DOCS\\BROKEN", fixStringRand(U"_______   there's something wrong with   _____________, "
                              //"ever since updating the OS, this weird driver started loading, '666.sys', "
                                "ever since   _______________,   this   _______________________________________, "
                              //"I can't find it on the '\\SYSTEM\\DRV\\' folder. I tried asking Kev if he knew more, " 
                                "I can't   ______________   '\\SYSTEM\\DRV\\'   _________   tried asking   _________   knew   ______"
                              //"but when i tried showing him, all he saw on the monitor was a blank screen..."
                                "___  when i tried  ____________________________________________  blank screen...", U'_', 123)},
};

std::map<std::string, std::string> textFiles
{
    {"\\HOME\\DOCS\\BROKEN", "I think there's something wrong with this computer, "
                 "ever since updating the   OS, this weird driver started loading, '666.sys'. "
                 "I can't find it on the      '\\SYSTEM\\DRV\\' folder. I tried asking Kev if he knew more, "
                 "but when i tried   showing him, all he saw on the monitor was a blank screen..."},
                 
    //{"\\DELETEME", ""},
    {"\\HOME\\README", "If i've forgotten my passwords, here's a hint: birthday DDMMYY"},
    {"\\HOME\\DOCS\\DATES", "Me - 24/07/67                                                                 "
                            "Noelle - 08/02/65                                                             "
                            "Abigail - 14/04/89                                                            "
    },
    {"\\HOME\\SENSITIVE\\IF_YOU_FOUND_THIS", "If you found this computer, please destroy it, i couldn't bear to             do it myself..."},
};

std::map<std::string, CommandProc> programs
{
    {"CD", &Command::Cd},
    {"DIR", &Command::Dir},
    {"LS", &Command::Dir},
    {"CLS", &Command::Clear},
    {"CLEAR", &Command::Clear},
    {"HELP", &Command::Help},
    {"READ", &Command::Read},
    {"CAT", &Command::Read},
    {"INSTALL", &Command::Install},
    {"UNLOCK", &Command::Unlock},
    {"RECOVERY", &Command::Recovery},
    //{"DECRYPT", &Command::Decrypt}, // TODO
    {"666", &Command::EndJamBuild},
};

std::map<std::vector<std::string>, program_help> programHelp
{
    {{"HELP"}, {"Show Help for Commands", {"HELP"_sv, "HELP <COMMAND>"_sv}}},
    {{"CD"}, {"Change directories or show the path of the current directory (use '..' to go back to the previous directory)", "CD <DIR>"}},
    {{"DIR", "LS"}, {"List entries of the current directory", {"LS", "LS <PATH>", "DIR", "DIR <PATH>"}}},
    {{"CLS", "CLEAR"}, {"Clear screen", {"CLS"_sv, "CLEAR"_sv}}},
    {{"CAT", "READ"}, {"Read text documents", {"CAT <PATH>"_sv, "READ <PATH>"_sv}}},
    {{"INSTALL"}, {"Install a program to your \\BIN\\ directory, allowing you to use it anywhere", "INSTALL <PATH>"}},
    {{"UNLOCK"}, {"Pass it a path and a password to unlock encrypted folders/files", "UNLOCK <PATH> <PASSWORD>"}},
    {{"RECOVERY"}, {"This can recover deleted files in the current folder", "RECOVERY"}},
    {{"666"}, program_help::hide({stringRand("________________________________________________________________________________________________________________________________________", '_', 819), stringRand("_________________", '_', 820)})},
};

std::map<std::string, std::map<std::string, std::map<std::string, dir_entry>>> directories
{
    {"C", {
        {"\\", {
            {"SYSTEM", {"SYSTEM", FOLDER}},
            {"BIN", {"BIN", FOLDER, FORBIDDEN}},
            {"HOME", {"HOME", FOLDER}},
            //{"DELETEME", {"DELETEME", TEXT, DELETED}},
        }},
        {"\\SYSTEM\\",{
            {"OS", {"OS", FOLDER, FORBIDDEN}},
            {"CFG", {"CFG", FOLDER, FORBIDDEN}},
            {"DRV", {"DRV", FOLDER}},
        }},
        {"\\SYSTEM\\OS\\",{
            
        }},
        {"\\SYSTEM\\CFG\\",{
            
        }},
        {"\\SYSTEM\\DRV\\",{
            {"PowerMgmt",{"PowerMgmt", DRIVER}},
            {"Display",{"Display", DRIVER}},
            {"Console",{"Console", DRIVER}},
            {"ExtMemory",{"ExtMemory", DRIVER}},
            {"SndBurst",{"SndBurst", DRIVER}},
            {"Command",{"Command", DRIVER}},
            {"Mouse",{"Mouse", DRIVER}},
            {"Printer",{"Printer", DRIVER}},
            {"Network",{"Network", DRIVER}},
            //{"666",{"666", DRIVER, HIDDEN}},
            {"666",{"666", PROGRAM, DELETED}},
        }},
        {"\\BIN\\",{
            {"CD", {"CD", PROGRAM}},
            {"CLS", {"CLS", PROGRAM}},
            {"CLEAR", {"CLEAR", PROGRAM}},
            {"DIR", {"DIR", PROGRAM}},
            {"LS", {"LS", PROGRAM}},
            {"HELP", {"HELP", PROGRAM}},
            //{"MAN", {"MAN", PROGRAM}},
            {"READ", {"READ", PROGRAM}},
            {"CAT", {"CAT", PROGRAM}},
            {"INSTALL", {"INSTALL", PROGRAM}},
            //{"UNLOCK", {"UNLOCK", PROGRAM}},
            //{"RECOVERY", {"RECOVERY", PROGRAM}},
            //{"DECRYPT", {"DECRYPT", PROGRAM}},
        }},
        {"\\HOME\\",{
            {"DOCS", {"DOCS", FOLDER}},
            {"SENSITIVE", {"SENSITIVE", FOLDER, ENCRYPTED, "140489"}},
            {"README", {"README", TEXT}},
        }},
        {"\\HOME\\DOCS\\",{
            {"BROKEN", {"BROKEN", TEXT, CORRUPTED}},
            {"DATES", {"DATES", TEXT}},
            {"UNLOCK", {"UNLOCK", PROGRAM}},
        }},
        {"\\HOME\\SENSITIVE\\",{
            {"IF_YOU_FOUND_THIS", {"IF_YOU_FOUND_THIS", TEXT}},
            {"RECOVERY", {"RECOVERY", PROGRAM}},
        }},
    }},
};
