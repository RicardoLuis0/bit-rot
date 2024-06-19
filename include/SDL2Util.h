#pragma once

#include <SDL2/SDL.h>
#include "SDL_mixer.h"


extern __thread char buf[1024];

inline std::string errMsg()
{
    return SDL_GetErrorMsg(buf, 1024);
}

inline bool operator == (const SDL_Keysym & ks, const SDL_Keycode &sym)
{
    return ks.sym == sym;
}

inline bool operator == (const SDL_Keysym & ks, const SDL_Scancode &scan)
{
    return ks.scancode == scan;
}

inline bool operator == (const SDL_KeyboardEvent & ke, const SDL_Keycode &sym)
{
    return ke.keysym.sym == sym;
}

inline bool operator == (const SDL_KeyboardEvent & ke, const SDL_Scancode &scan)
{
    return ke.keysym.scancode == scan;
}
