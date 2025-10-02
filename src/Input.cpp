#include "Input.h"
#include "Log.h"
#include "SDL2Util.h"
#include "Renderer.h"

#include <stdexcept>
#include <map>

void Input::Init() {}

int shiftCount = 0;
int ctrlCount = 0;

bool Input::ShiftPressed()
{
    return shiftCount > 0;
}

bool Input::CtrlPressed()
{
    return ctrlCount > 0;
}

bool Input::Poll(Responder fn)
{
    SDL_Event e;
    while(SDL_PollEvent(&e) != 0)
    {
        switch(e.type)
        {
        case SDL_WINDOWEVENT:
            if(e.window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
            {
                Renderer::UpdateResolution(e.window.data1, e.window.data2);
            }
            break;
        case SDL_QUIT:
            return false;
            break;
        case SDL_KEYUP:
        case SDL_KEYDOWN:
            if((e.key == SDLK_LSHIFT || e.key == SDLK_RSHIFT) && e.key.repeat == 0)
            {
                if(e.type == SDL_KEYUP)
                {
                    shiftCount--;
                }
                else
                {
                    shiftCount++;
                }
            }
            else if((e.key == SDLK_LCTRL || e.key == SDLK_RCTRL) && e.key.repeat == 0)
            {
                if(e.type == SDL_KEYUP)
                {
                    ctrlCount--;
                }
                else
                {
                    ctrlCount++;
                }
            }
            [[fallthrough]];
        case SDL_TEXTINPUT: 
        default:
            if(fn) fn(&e);
        }
    }
    return true;
}
