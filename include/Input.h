#pragma once

#include <string>

union SDL_Event;

namespace Input
{
    using Responder = void(*)(SDL_Event*);
    
    
    bool ShiftPressed();
    bool CtrlPressed();
    void Init();
    inline void Quit() {};
    bool Poll(Responder);
}
