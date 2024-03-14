
#include "Log.h"
#include "System.h"

#include <stdexcept>
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

//static COORD lastConSize;

void System::Init()
{
    LogDebug("Active Code Page: %u", GetACP());
    conIn = GetStdHandle(STD_INPUT_HANDLE);
    conOut = GetStdHandle(STD_OUTPUT_HANDLE);
    conErr = GetStdHandle(STD_ERROR_HANDLE);
    /*
    if(!GetConsoleMode(conIn, &prevConInMode))
    {
        throw std::runtime_error(GetLastErrorStr());
    }
    if(!GetConsoleMode(conOut, &prevConOutMode))
    {
        throw std::runtime_error(GetLastErrorStr());
    }
    */
    /*
    if(!SetConsoleMode(conIn, ENABLE_MOUSE_INPUT | ENABLE_EXTENDED_FLAGS | ENABLE_WINDOW_INPUT))
    {
        std::string err = GetLastErrorStr();
        LogCritical(err);
        throw std::runtime_error(err);
    }
    */
    /*
    if(!SetConsoleMode(conOut, ENABLE_PROCESSED_OUTPUT | ENABLE_PROCESSED_OUTPUT | ENABLE_LVB_GRID_WORLDWIDE))
    {
        throw std::runtime_error(GetLastErrorStr());
    }
    */
}

//std::string eventType(
void System::ReadConsoleRaw(std::string &buffer)
{
    /*
    DWORD num;
    if(!GetNumberOfConsoleInputEvents(conIn, &num))
    {
        std::string err = GetLastErrorStr();
        LogCritical(err);
        throw std::runtime_error(err);
    }
    
    if(num > 0)
    {
        INPUT_RECORD events[num];
        DWORD numRead;
        if(!ReadConsoleInput(conIn, events, num, &numRead))
        {
            std::string err = GetLastErrorStr();
            LogCritical(err);
            throw std::runtime_error(err);
        }
        for(unsigned i = 0; i < numRead; i++)
        {
            //Log::LogVerbose("INPUT_RECORD type = %u", events[i].EventType);
            switch(events[i].EventType)
            {
            case KEY_EVENT:
                //Log::LogVerbose("KEY_EVENT");
                break;
            case MOUSE_EVENT:
                //Log::LogVerbose("MOUSE_EVENT");
                break;
            default:
                break;
            }
        }
    }
    */
}

void System::Quit()
{
    /*
    SetConsoleMode(conOut, prevConOutMode);
    SetConsoleMode(conIn, prevConInMode);
    */
}
