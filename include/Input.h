#pragma once

#include <string>

union SDL_Event;

namespace Audio
{
    void Init();
    
    
    //samples
    void StartFan();
    void Beep();
    void Error();
    
    bool ErrorPlaying();
    
    //music
    void FadeMusic(int ms);
    void PlayMusic(const std::string &name);
    
    inline void Quit() {};
}

namespace Input
{
    using Responder = void(*)(SDL_Event*);
    
    
    bool ShiftPressed();
    bool CtrlPressed();
    void Init();
    inline void Quit() {};
    bool Poll(Responder);
}
