#pragma once

#include <string>
#include <vector>

namespace Command
{
    int Exit(const std::vector<std::string> &args);
    int Cd(const std::vector<std::string> &args);
    int Dir(const std::vector<std::string> &args);
    int Clear(const std::vector<std::string> &args);
    int Help(const std::vector<std::string> &args);
    int Read(const std::vector<std::string> &args);
    int Recovery(const std::vector<std::string> &args);
    int Decrypt(const std::vector<std::string> &args);
    int Install(const std::vector<std::string> &args);
    int Unlock(const std::vector<std::string> &args);
    int EndJamBuild(const std::vector<std::string> &args);
}
