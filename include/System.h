#pragma once

#include <string>

namespace System
{
    void Init();
    void ReadConsoleRaw(std::string &buffer);
    void Quit();
}
