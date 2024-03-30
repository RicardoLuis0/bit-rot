
#include "Log.h"
#include "System.h"

#include <stdexcept>

#ifdef _WIN32

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#undef ERROR

HANDLE conIn;
HANDLE conOut;
HANDLE conErr;

DWORD prevConInMode;
DWORD prevConOutMode;

static std::string GetErrorStr(DWORD dwErr)
{
    LPSTR lpBuffer;
    FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, nullptr, dwErr, 0, (LPSTR) &lpBuffer, 1, nullptr);
    std::string out(lpBuffer);
    LocalFree(lpBuffer);
    return out;
}

[[maybe_unused]]
static std::string GetLastErrorStr()
{
    return GetErrorStr(GetLastError());
}
#endif

void System::Init()
{
#ifdef _WIN32
    LogDebug("Active Code Page: %u", GetACP());
#endif
}

void System::ReadConsoleRaw(std::string &buffer)
{
}

void System::Quit()
{
}
