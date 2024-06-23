#pragma once

#include "Common.h"

namespace SaveData
{
    
    enum SaveActionType
    {
        UNLOCK,
        INSTALL,
        RECOVERY,
        SAVE_ACTION_TYPE_COUNT,
    };
    
    struct SaveAction
    {
        SaveActionType type;
        std::string info;
        std::string extra_info;
    };
    
    void Init();
    void Quit();
    void Reset();
    void Clear();
    
    bool HasSave();
    void MarkNewGameOk();
    
    void SetFolder(std::string_view folder);
    
    void PushAction(SaveActionType type, std::string_view info, std::string_view extra_info = "");
    
    void PushHistory(std::string_view cmd);
    void PushBuffer(std::string_view text, std::span<const uint8_t> props);
    
    void ClearBuffer();
    
    void GetSave(std::string &savePath, std::vector<SaveAction> &actions, std::vector<std::string> &history, std::vector<std::pair<std::string, std::vector<uint8_t>>> &buffer);
};
