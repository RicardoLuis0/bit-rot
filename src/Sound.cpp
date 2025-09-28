#include "Sound.h"
#include "Common.h"

#include "SDL2Util.h"
#include "SDL_mixer.h"

#include <filesystem>
#include <vector>
#include <map>

std::vector<Mix_Music *> loaded_music;
std::vector<Mix_Chunk *> loaded_sfx;

std::map<std::string, int> music_names;
std::map<std::string, int> sfx_names;

constexpr int CHAN_MAX = 65535;
constexpr int CHAN_COUNT = 65536;

constexpr int looping_sfx_offset = 55535; // "only" allows 10k unique sound effects

static void chanDone(int chan, int interrupted)
{
    if(!interrupted)
    {
        int tag = Mix_GetChannelGroup(chan);
        
        if(tag >= 0)
        {
            //Mix_PlayChannelSetTag(tag + looping_sfx_offset, loaded_sfx[tag], -1, -1);
            Mix_PlayChannelSetTag(chan, loaded_sfx[tag], -1, -1);
        }
    }
}


void Sound::Init()
{
    if(Mix_Init(MIX_INIT_OGG) != MIX_INIT_OGG)
    {
        throw FatalError(errMsg()); // technically this should use Mix_GetError, but that's just a #define for SDL_GetError, so it's fine
    }
    
    if(Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) != 0)
    {
        throw FatalError(errMsg());
    }
    
    if(Mix_AllocateChannels(CHAN_COUNT) != CHAN_COUNT)
    {
        throw FatalError(errMsg());
    }
    
    auto it = std::filesystem::recursive_directory_iterator("Data");
    
    for(const std::filesystem::directory_entry &entry : it)
    {
        if(entry.is_regular_file())
        {
            std::string extension = Util::StrToLower(entry.path().extension().string());
            if(extension == ".ogg")
            {
                std::string filepath = entry.path().string();
                std::string name = Util::StrToLower(entry.path().stem().string());
                LogDebug("Loading Music "+Util::QuoteString(name)+" From File: "+Util::QuoteString(entry.path().string()));
                Mix_Music * mus = Mix_LoadMUS(filepath.c_str());
                music_names.insert({name, loaded_music.size()});
                loaded_music.push_back(mus);
            }
            else if(extension == ".wav")
            {
                std::string filepath = entry.path().string();
                std::string name = Util::StrToLower(entry.path().stem().string());
                LogDebug("Loading Sample "+Util::QuoteString(name)+" From File: "+Util::QuoteString(entry.path().string()));
                Mix_Chunk * sfx = Mix_LoadWAV(filepath.c_str());
                sfx_names.insert({name, loaded_sfx.size()});
                loaded_sfx.push_back(sfx);
            }
        }
    }
    
    Mix_ChannelFinished(chanDone);
    
    LogDebug("Music/Samples loaded");
}

void Sound::Quit()
{
    for(auto &music : loaded_music)
    {
        Mix_FreeMusic(music);
    }
    for(auto &sfx : loaded_sfx)
    {
        Mix_FreeChunk(sfx);
    }
    loaded_music.clear();
    loaded_sfx.clear();
    music_names.clear();
    sfx_names.clear();
    Mix_Quit();
}

void Sound::FadeMusic(int ms)
{
    Mix_FadeOutMusic(ms);
}

static int currentChannel = 0;

void Sound::PlayMusic(const std::string &name, int crossfadeMs)
{
    std::string n = Util::StrToLower(name);
    auto index = music_names.find(n);
    
    if(index != music_names.end())
    {
        if(Mix_PlayingMusicChannel(currentChannel))
        { // crossfade
            Mix_FadeOutMusicChannel(currentChannel, crossfadeMs);
            currentChannel = (currentChannel + 1) % 2;
        }
        Mix_FadeInMusic(loaded_music[index->second], currentChannel, -1, crossfadeMs);
    }
    else
    {
        LogWarn("cannot find music named "+Util::QuoteString(n));
    }
}


void Sound::PlaySFX(const std::string &name, bool overlap, bool loop)
{
    std::string n = Util::StrToLower(name);
    auto index = sfx_names.find(n);
    
    if(overlap && loop)
    {
        LogWarn("cannot loop an overlapped sound");
    }
    else if(index != sfx_names.end())
    {
        int i = index->second;
        
        Mix_PlayChannelSetTag(overlap ? -1 : (i + looping_sfx_offset), loaded_sfx[i], loop ? -1 : 0, -1);
    }
    else
    {
        LogWarn("cannot find sound named "+Util::QuoteString(n));
    }
}

void Sound::PlaySFXStartLoop(const std::string &start, const std::string &loop)
{
    std::string s = Util::StrToLower(start);
    std::string l = Util::StrToLower(loop);
    auto index_s = sfx_names.find(s);
    auto index_l = sfx_names.find(l);
    
    if(index_s != sfx_names.end())
    {
        if(index_l != sfx_names.end())
        {
            int i = index_s->second;
            int i2 = index_l->second;
            
            Mix_PlayChannelSetTag(i2 + looping_sfx_offset, loaded_sfx[i], 0, i2);
        }
        else
        {
            LogWarn("cannot find sound named "+Util::QuoteString(l));
        }
    }
    else
    {
        LogWarn("cannot find sound named "+Util::QuoteString(s));
    }
}

bool Sound::IsSamplePlaying(const std::string &name)
{
    std::string n = Util::StrToLower(name);
    auto index = sfx_names.find(n);
    if(index != sfx_names.end())
    {
        return Mix_Playing(index->second + looping_sfx_offset);
    }
    else
    {
        LogWarn("cannot find sound named "+Util::QuoteString(n));
    }
    return false;
}

void Sound::StopSample(const std::string &name)
{
    std::string n = Util::StrToLower(name);
    auto index = sfx_names.find(n);
    if(index != sfx_names.end())
    {
        if(Mix_Playing(index->second + looping_sfx_offset))
        {
            Mix_HaltChannel(index->second + looping_sfx_offset);
        }
    }
    else
    {
        LogWarn("cannot find sound named "+Util::QuoteString(n));
    }
}
