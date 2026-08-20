#include "SaveData.h"
#include "Json.h"
#include "Config.h"

#include <filesystem>



JSON::Element saveData;

bool save_ok = false;

constexpr const char * saveFilePlain = "saveData.json";
constexpr const char * saveFileCompressed = "saveData.bin";

int currentSaveVersion = 1;

void SaveData::Reset()
{
    saveData = JSON::Object({
        {"SavedPath",    JSON::String("\\")},
        {"SavedActions", JSON::Array({})},
        {"SavedHistory", JSON::Array({})},
        {"SavedBuffer",  JSON::Array({})}
    });
    
    save_ok = false;
}

void SaveData::Clear()
{
    Reset();
    
    if(std::filesystem::exists(saveFilePlain))
    {
        std::filesystem::remove(saveFilePlain);
    }
    
    if(std::filesystem::exists(saveFileCompressed))
    {
        std::filesystem::remove(saveFileCompressed);
    }
}

void SaveData::MarkNewGameOk()
{
    save_ok = (!std::filesystem::exists(saveFilePlain) && !std::filesystem::exists(saveFileCompressed));
}

void SaveData::FixUpOldSave()
{
    int oldVersion = 0;
    if(saveData.contains("SaveVersion"))
    {
        oldVersion = saveData["SaveVersion"].get_int();
    }
    
    bool hasMessages = false;
    bool unlockRenamed = false;
    
    if(oldVersion < 1)
    {
        if(saveData.contains("SavedActions"))
        {
            if(saveData["SavedActions"].is_arr())
            {
                for(auto &saveAction : saveData["SavedActions"].get_arr())
                {
                    if(!saveAction.contains("Type") || !saveAction["Type"].is_int() || !saveAction.contains("Info") || !saveAction["Info"].is_str()) continue;
                    if(saveAction["Type"].get_int() == SaveData::INSTALL && saveAction["Info"].get_str() == "UNLOCK")
                    {
                        saveAction.set("Info", "DECRYPT");
                        unlockRenamed = true;
                        hasMessages = true;
                    }
                }
            }
        }
    }
    
    // info messages
    if(hasMessages)
    {
        PushBuffer("", {});
        PushBuffer(     fixString(U"╔═════════════════════════════════════════════════════════════════════════════╗"), repeat_array<uint8_t, 80, 0x1>());
        PushBuffer(     fixString(U"║                        Since last time you've played:                       ║"), repeat_array<uint8_t, 80, 0x1>());
        if(unlockRenamed)
        {
            PushBuffer( fixString(U"║                                                                             ║"), repeat_array<uint8_t, 80, 0x1>());
            PushBuffer( fixString(U"║                      UNLOCK has been renamed to DECRYPT                     ║"), repeat_array<uint8_t, 80, 0x1>());
        }
        PushBuffer(     fixString(U"╚═════════════════════════════════════════════════════════════════════════════╝"), repeat_array<uint8_t, 80, 0x1>());
        PushBuffer("", {});
    }
}

//TODO: gzip save data to reduce file size
void SaveData::Init()
{
    if(std::filesystem::exists(saveFilePlain) || std::filesystem::exists(saveFileCompressed))
    {
        try
        {
            if(std::filesystem::exists(saveFileCompressed) && (!std::filesystem::exists(saveFilePlain) || std::filesystem::last_write_time(saveFileCompressed) > std::filesystem::last_write_time(saveFilePlain)))
            {
                saveData = JSON::Parse(Util::Decompress(Util::ReadFileBinary(saveFileCompressed)));
            }
            else
            {
                saveData = JSON::Parse(Util::ReadFile(saveFilePlain));
            }
            
            saveData.get_obj();
            FixUpOldSave();
            saveData.set("SaveVersion", JSON::Int(currentSaveVersion));
        }
        catch(JSON::JSON_Exception &e)
        {
            throw FatalError("Malformed JSON in Save Data: "+e.msg_top);
        }
        
        save_ok = true;
    }
    else
    {
        saveData = JSON::Object({
            {"SavedPath",    JSON::String("\\")},
            {"SavedActions", JSON::Array({})},
            {"SavedHistory", JSON::Array({})},
            {"SavedBuffer",  JSON::Array({})},
            {"SaveVersion",  JSON::Int(currentSaveVersion)}
        });
    }
}

void SaveData::Quit()
{
    if(save_ok)
    {
        if(Config::getBoolOr("CompressSaves", DefaultCompressSaves))
        {
            Util::WriteFileBinary(saveFileCompressed, Util::Compress(saveData.to_json_min()));
        }
        else
        {
            Util::WriteFile(saveFilePlain, saveData.to_json());
        }
    }
}

bool SaveData::HasSave()
{
    return save_ok;
}


void SaveData::PushAction(SaveActionType type, std::string_view info, std::string_view extra_info)
{
    saveData["SavedActions"].get_arr().emplace_back(JSON::Object({
        {"Type", JSON::Int(int(type))},
        {"Info", JSON::String(std::string(info))},
        {"ExtraInfo", JSON::String(std::string(extra_info))}
    }));
    save_ok = true;
}

void SaveData::PushHistory(std::string_view cmd)
{
    saveData["SavedHistory"].get_arr().insert(saveData["SavedHistory"].get_arr().begin(), JSON::String(std::string(cmd)));
    save_ok = true;
}

void SaveData::PushBuffer(std::string_view text, std::span<const uint8_t> props)
{
    saveData["SavedBuffer"].get_arr().emplace_back(JSON::Object({
        {"Text", JSON::String(std::string(text))},
        {"Props", JSON::Array(Util::Map(props, JSON::Int))}
    }));
    save_ok = true;
}

void SaveData::ClearBuffer()
{
    saveData["SavedBuffer"].get_arr().clear();
    save_ok = true;
}

void SaveData::SetFolder(std::string_view folder)
{
    saveData["SavedPath"] = JSON::String(std::string(folder));
    save_ok = true;
}

void SaveData::SetConsoleVars(std::map<std::string, std::string> &vars)
{
    if(!saveData.get_obj().contains("SavedVars"))
    {
        saveData.get_obj().insert({"SavedVars", JSON::Object({})});
    }
    saveData["SavedVars"].get_obj().clear();
    for(auto &var : vars)
    {
        saveData["SavedVars"].get_obj().insert({var.first, JSON::String(var.second)});
    }
    save_ok = true;
}

std::map<std::string, std::string> SaveData::GetConsoleVars()
{
    std::map<std::string, std::string> vars;
    
    if(saveData.get_obj().contains("SavedVars")) for(auto &var : saveData["SavedVars"].get_obj())
    {
        vars[var.first] = var.second.get_str();
    }
    return vars;
}

void SaveData::GetSave(std::string &savePath, std::vector<SaveData::SaveAction> &actions, std::vector<std::string> &history, std::vector<std::pair<std::string, std::vector<uint8_t>>> &buffer) try
{
    savePath = saveData["SavedPath"].get_str();
    
    actions.clear();
    history.clear();
    buffer.clear();
    
    for(auto &action : saveData["SavedActions"].get_arr())
    {
        int type = action["Type"].get_int();
        if(type < 0 || type >= SAVE_ACTION_TYPE_COUNT)
        {
            throw FatalError("Malformed Save Data: Invalid Action Type");
        }
        std::string info = action["Info"].get_str();
        std::string extra_info = action["ExtraInfo"].get_str();
        actions.emplace_back(SaveActionType(type), info, extra_info);
    }
    
    for(auto &hist : saveData["SavedHistory"].get_arr())
    {
        history.push_back(hist.get_str());
    }
    
    for(auto &line : saveData["SavedBuffer"].get_arr())
    {
        buffer.push_back({line["Text"].get_str(),Util::Map(line["Props"].get_arr(), &JSON::Element::get_uint8)});
    }
}
catch(JSON::JSON_Exception &e)
{
    throw FatalError("Malformed JSON in Save Data: "+e.msg_top);
}
