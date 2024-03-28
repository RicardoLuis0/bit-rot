#pragma once

#include <string>
#include <vector>

namespace Command
{
    void Exit(const std::vector<std::string> &args);
    void Cd(const std::vector<std::string> &args);
    void Dir(const std::vector<std::string> &args);
    void Clear(const std::vector<std::string> &args);
    void Help(const std::vector<std::string> &args);
    void Read(const std::vector<std::string> &args);
    void Recovery(const std::vector<std::string> &args);
    void Decrypt(const std::vector<std::string> &args);
    void Install(const std::vector<std::string> &args);
    void Unlock(const std::vector<std::string> &args);
    void EndJamBuild(const std::vector<std::string> &args);
}
