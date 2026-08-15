#include "Game.h"
#include "Common.h"
#include "Json.h"
#include "Input.h"
#include "Renderer.h"
#include "SDL2Util.h"
#include "Command.h"
#include "GameData.h"
#include <memory>
#include <filesystem>


#ifdef _WIN32
#include "zip.h"

unsigned char gamedatazip[]
{
    #embed "../gamedata.zip"
};
#endif


constexpr const char * fontInfoFile = "Fonts/FontInfo.json";

std::string fontInfo;
std::map<std::string, std::unique_ptr<GameData::ImageData>> font_files;


#ifdef _WIN32
zip_source_t * zip_source;
zip_t * zip_archive;

bool zip_file_exists(std::string filename)
{
    return zip_name_locate(zip_archive, filename.c_str(), ZIP_FL_NOCASE) >= 0;
}

std::string read_zip_file(std::string filename)
{
    zip_stat_t stat;
    zip_stat_init(&stat);
    if(zip_stat(zip_archive, filename.c_str(), ZIP_FL_NOCASE, &stat) != 0)
    {
        throw FatalError(std::string("failed to stat ")+filename+" in embedded resources: "+zip_strerror(zip_archive));
    }
    else if(!(stat.valid&ZIP_STAT_SIZE))
    {
        throw FatalError(std::string("size not available for ")+filename+" in embedded resources");
    }
    else
    {
        zip_file_t * f = zip_fopen_index(zip_archive, stat.index, 0);
        if(f)
        {
            std::string r;
            r.resize(stat.size);
            zip_fread(f, r.data(), stat.size);
            zip_fclose(f);
            return r;
        }
        else
        {
            throw FatalError(std::string("failed to open ")+filename+" in embedded resources: "+zip_strerror(zip_archive));
        }
    }
}

std::vector<std::byte> read_zip_file_binary(std::string filename)
{
    zip_stat_t stat;
    zip_stat_init(&stat);
    if(zip_stat(zip_archive, filename.c_str(), ZIP_FL_NOCASE, &stat) != 0)
    {
        throw FatalError(std::string("failed to stat ")+filename+" in embedded resources: "+zip_strerror(zip_archive));
    }
    else if(!(stat.valid&ZIP_STAT_SIZE))
    {
        throw FatalError(std::string("size not available for ")+filename+" in embedded resources");
    }
    else
    {
        zip_file_t * f = zip_fopen_index(zip_archive, stat.index, 0);
        if(f)
        {
            std::vector<std::byte> r;
            r.resize(stat.size);
            zip_fread(f, r.data(), stat.size);
            zip_fclose(f);
            return r;
        }
        else
        {
            throw FatalError(std::string("failed to open ")+filename+" in embedded resources: "+zip_strerror(zip_archive));
        }
    }
}

void open_zip_file()
{
    zip_error_t err;
    zip_error_init(&err);
    
    zip_source=zip_source_buffer_create(gamedatazip, sizeof(gamedatazip), 0, &err);
    
    if(!zip_source)
    {
        throw FatalError(std::string("Failed to open embedded resources: ")+zip_error_strerror(&err));
    }
    
    zip_archive = zip_open_from_source(zip_source, ZIP_RDONLY, &err);
    
    if(!zip_archive)
    {
        throw FatalError(std::string("Failed to open embedded resources: ")+zip_error_strerror(&err));
    }
}

std::vector<std::string> GameData::ListEmbeddedFiles()
{
    std::vector<std::string> tmp;
    int n = zip_get_num_entries(zip_archive, 0);
    for(int i = 0; i < n; i++)
    {
        zip_stat_t stat;
        zip_stat_init(&stat);
        if(zip_stat_index(zip_archive, i, 0, &stat) != 0)
        {
            throw FatalError("failed to stat index "+std::to_string(i)+" in embedded resources: "+zip_strerror(zip_archive));
        }
        tmp.push_back(stat.name);
    }
    return tmp;
}

#else
#define zip_file_exists(...) false
#define read_zip_file(...) ""
#define read_zip_file_binary(...) (std::vector<std::byte>{})
#define open_zip_file(...)

std::vector<std::string> GameData::ListEmbeddedFiles()
{
    return {};
}

#endif

std::string GameData::GetFontInfo()
{
    return fontInfo;
}

bool GameData::Exists(std::string filename)
{
    return std::filesystem::exists(filename) || zip_file_exists(filename);
}

void GameData::Init()
{
    open_zip_file();
    if(std::filesystem::exists(fontInfoFile))
    {
        fontInfo = Util::ReadFile(fontInfoFile);
    }
    else if(zip_file_exists(fontInfoFile))
    {
        fontInfo = read_zip_file(fontInfoFile);
    }
    else
    {
        throw FatalError(std::string(fontInfoFile)+" is missing");
    }
}

GameData::ImageData* GameData::GetFont(std::string fontName)
{
    if(auto it = font_files.find(fontName); it != font_files.end())
    {
        return it->second.get();
    }
    else if(std::filesystem::exists(fontName))
    {
        uint32_t width, height;
        std::vector<uint32_t> image = Util::ReadFileBitmap(fontName, width, height);
        ImageData * img = new ImageData {std::move(image), width, height};
        font_files.emplace(fontName, img);
        return img;
    }
    else if(zip_file_exists(fontName))
    {
        auto data = read_zip_file_binary(fontName);
        uint32_t width, height;
        std::vector<uint32_t> image = Util::ReadBitmap(fontName, data, width, height);
        ImageData * img = new ImageData {std::move(image), width, height};
        font_files.emplace(fontName, img);
        return img;
    }
    else
    {
        throw FatalError(std::string(fontInfoFile)+" is missing");
    }
}

std::string GameData::ReadFile(std::string filename)
{
    if(std::filesystem::exists(filename))
    {
        return Util::ReadFile(filename);
    }
    else if(zip_file_exists(filename))
    {
        return read_zip_file(filename);
    }
    else
    {
        throw FatalError(std::string("could not find file ")+filename);
    }
}

std::vector<std::byte> GameData::ReadFileBinary(std::string filename)
{
    if(std::filesystem::exists(filename))
    {
        return Util::ReadFileBinary(filename);
    }
    else if(zip_file_exists(filename))
    {
        return read_zip_file_binary(filename);
    }
    else
    {
        throw FatalError(std::string("could not find file ")+filename);
    }
}

void GameData::Quit()
{
    
}

using enum dir_entry_type;
using enum hide_type;

std::string currentDrive = "C";
std::string currentFolder = "\\";

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
    {"EXIT", &Command::Exit},
    //{"DECRYPT", &Command::Decrypt}, // TODO
    {"666", &Command::EndJamBuild},
};

std::map<std::vector<std::string>, program_help> programHelp
{
    {{"HELP"}, {"Show Help for Commands", {"HELP", "HELP <COMMAND>"}}},
    {{"CD"}, {"Change directories or show the path of the current directory (use '..' to go back to the previous directory)", "CD <DIR>"}},
    {{"DIR", "LS"}, {"List entries of the current directory", {"LS", "LS <PATH>", "DIR", "DIR <PATH>"}}},
    {{"CLS", "CLEAR"}, {"Clear screen", {"CLS", "CLEAR"}}},
    {{"CAT", "READ"}, {"Read text documents", {"CAT <PATH>", "READ <PATH>"}}},
    {{"INSTALL"}, {"Install a program to your \\BIN\\ directory, allowing you to use it anywhere", "INSTALL <PATH>"}},
    {{"UNLOCK"}, {"Pass it a path and a password to unlock encrypted folders/files", "UNLOCK <PATH> <PASSWORD>"}},
    {{"RECOVERY"}, {"This can recover deleted files in the current/the specified folder", {"RECOVERY", "RECOVERY <DIR>"}}},
    {{"EXIT"}, {"Save and Exit", "EXIT"}},
    {{"666"},
        program_help::hide(
            {
                stringRandDyn("________________________________________________________________________________________________________________________________________", '_'),
                {
                    //             666
                    stringRandDyn("___", '_'),
                    //             666 <????>
                    stringRandDyn("___ <____>", '_'),
                    //             666 <????> <???????>
                    stringRandDyn("___ <____> <_______>", '_'),
                }
            }
        )},
};

std::map<std::string, std::map<std::string, std::map<std::string, dir_entry>>> directories;

std::string JoinText(JSON::Element e)
{
    if(e.is_arr())
    {
        return Util::Join(Util::Map(e.get_arr(), &JoinText), "");
    }
    else
    {
        return e.get_str();
    }
}

std::map<std::string, std::string> textFilesCorrupted;
std::map<std::string, std::string> textFiles;
std::map<std::string, std::string> texts;

std::map<std::string, dir_entry_type> dir_entry_types
{
    {"FOLDER",FOLDER},
    {"TEXT", TEXT},
    {"DATA", DATA},
    {"PROGRAM", PROGRAM},
    {"PROGRAM_ALIAS", PROGRAM_ALIAS},
    {"DRIVER", DRIVER},
};
std::map<std::string, hide_type> hide_types
{
    {"VISIBLE", VISIBLE},
    {"CORRUPTED", CORRUPTED},
    //{"ENCRYPTED", ENCRYPTED},
    {"DELETED", DELETED},
    {"HIDDEN", HIDDEN},
    {"FORBIDDEN", FORBIDDEN},
};

constexpr const char * dataFile = "GameData/data.json";

std::vector<initTextLine> initText;
uint32_t numRecoveryTexts;

void Game::LoadData() try
{
    JSON::Element e = JSON::Parse(GameData::ReadFile(dataFile));
    
    JSON::Element corruptedfiles = e["CorruptedTextFiles"];
    int corruptedseed = 123;
    for(auto &pair : corruptedfiles.get_obj())
    {
        textFilesCorrupted[pair.first] = stringRand(JoinText(pair.second), '_', corruptedseed++);
    }
    
    JSON::Element files = e["TextFiles"];
    for(auto &pair : files.get_obj())
    {
        textFiles[pair.first] = JoinText(pair.second);
    }
    
    JSON::Element game_texts = e["Texts"];
    for(auto &pair : game_texts.get_obj())
    {
        texts[pair.first] = JoinText(pair.second);
    }
    
    JSON::Element game_intro_texts = e["IntroTexts"];
    for(JSON::Element &line : game_intro_texts.get_arr())
    {
        initTextLine l;
        l.timer = line[0].get_int();
        l.text = line[1].get_str();
        l.beep = line[2].get_bool();
        l.recovery = line[3].get_bool();
        l.intro = line[4].get_bool();
        initText.push_back(std::move(l));
    }
    
    numRecoveryTexts = Util::Reduce<uint32_t>(initText,
        [](const initTextLine& line, uint32_t acc)
        {
            return (line.intro) ? acc : acc + 1;
        }
    );
    
    JSON::Element drives = e["Drives"];
    
    for(auto &drive : drives.get_obj())
    {
        std::map<std::string, std::map<std::string, dir_entry>> dirs = {};
        
        for(auto &folder : drive.second.get_obj())
        {
            std::map<std::string, dir_entry> entries;
            for(auto &files : folder.second.get_obj())
            {
                auto &arr = files.second.get_arr();
                switch(arr.size())
                {
                case 2:
                    //type
                    if(!dir_entry_types.contains(arr[1].get_str()))
                    {
                        throw FatalError("Invalid directory type in data.json");//TODO improve error
                    }
                    entries.emplace(std::pair{files.first, dir_entry(arr[0].get_str(), dir_entry_types[arr[1].get_str()])});
                    break;
                case 3:
                    //type + attr
                    if(!dir_entry_types.contains(arr[1].get_str()))
                    {
                        throw FatalError("Invalid directory type in data.json");//TODO improve error
                    }
                    if(!hide_types.contains(arr[2].get_str()))
                    {
                        throw FatalError("Invalid directory attribute in data.json");//TODO improve error
                    }
                    entries.emplace(std::pair{files.first, dir_entry(arr[0].get_str(), dir_entry_types[arr[1].get_str()], hide_types[arr[2].get_str()])});
                    break;
                case 4:
                    //type + encrypted attr + pass
                    if(!dir_entry_types.contains(arr[1].get_str()))
                    {
                        throw FatalError("Invalid directory type in data.json");//TODO improve error
                    }
                    if(arr[2].get_str() == "ENCRYPTED")
                    {
                        entries.emplace(std::pair{files.first, dir_entry(arr[0].get_str(), dir_entry_types[arr[1].get_str()], ENCRYPTED, arr[3].get_str())});
                        break;
                    }
                    [[fallthrough]];
                default:
                    throw FatalError("Invalid parameter count for directory in data.json");
                }
            }
            dirs.emplace(std::pair{folder.first, std::move(entries)});
        }
        
        directories.emplace(std::pair{drive.first, std::move(dirs)});
        
    }
}
catch(JSON::JSON_Exception &e)
{
    throw FatalError("Malformed JSON in "+Util::QuoteString(dataFile)+": "+e.msg_top);
}
