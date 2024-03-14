#include "Input.h"
#include "Log.h"
#include "SDL2Util.h"
#include "Renderer.h"

#include <SDL2/SDL_mixer.h>

#include <stdexcept>
#include <map>

Mix_Music * forest;
Mix_Music * lost;
Mix_Music * tension;
Mix_Music * loop;

Mix_Chunk * fan_start;
Mix_Chunk * fan_loop;
Mix_Chunk * beep;
Mix_Chunk * error;

std::map<std::string, Mix_Music**> music
{
    {"forest", &forest},
    {"lost", &lost},
    {"tension", &tension},
    {"loop", &loop},
};

#define CHANNEL_ERROR 125
#define CHANNEL_FAN_START 127
#define CHANNEL_FAN_LOOP 126

extern int introStage;
extern uint32_t introStartMs;
void chanDone(int chan)
{
    if(chan == CHANNEL_FAN_START)
    {
        Mix_PlayChannel(CHANNEL_FAN_LOOP, fan_loop, -1);
    }
    else if(chan == CHANNEL_ERROR)
    {
        introStage = 7;
        introStartMs = Util::MsTime();
    }
}

#define LOADMUS(name)\
    name = Mix_LoadMUS("Data/" #name ".ogg");\
    if(!name)\
    {\
        throw std::runtime_error(errMsg());\
    }\
    LogDebug("'Data/" #name ".ogg' Loaded");

#define LOADSFX(name)\
    name = Mix_LoadWAV("Data/" #name ".wav");\
    if(!name)\
    {\
        throw std::runtime_error(errMsg());\
    }\
    LogDebug("'Data/" #name ".wav' Loaded");

void Input::Init() {}
void Audio::Init()
{
    if(Mix_Init(MIX_INIT_OGG) != MIX_INIT_OGG)
    {
        throw std::runtime_error(errMsg()); // technically this should use Mix_GetError, but that's just a #define for SDL_GetError, so it's fine
    }
    
    if(Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) != 0)
    {
        throw std::runtime_error(errMsg());
    }
    
    if(Mix_AllocateChannels(128) != 128)
    {
        throw std::runtime_error(errMsg());
    }
    
    LOADMUS(forest);
    LOADMUS(lost);
    LOADMUS(tension);
    LOADMUS(loop);
    
    LOADSFX(fan_start);
    LOADSFX(fan_loop);
    LOADSFX(beep);
    LOADSFX(error);
    
    Mix_ChannelFinished(chanDone);
    
    LogDebug("Music/Samples loaded");
}

void Audio::StartFan()
{
    Mix_PlayChannel(CHANNEL_FAN_START, fan_start, 0);
}

void Audio::Beep()
{
    Mix_PlayChannel(-1, beep, 0);
}

void Audio::Error()
{
    Mix_PlayChannel(CHANNEL_ERROR, error, 0);
}

/*
void Audio::PauseFan()
{
    
}

void Audio::ResumeFan()
{
    
}

void Audio::StopFan()
{
    
}
*/

void Audio::FadeMusic(int ms)
{
    Mix_FadeOutMusic(ms);
}
void Audio::PlayMusic(const std::string &name)
{
    Mix_PlayMusic(*music.at(name), -1);
}

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
        default:
            if(fn) fn(&e);
        }
    }
    return true;
}
