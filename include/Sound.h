#pragma once

#include <string>

namespace Sound
{
    void Init();
    void Quit();
    
    void PlaySFX(const std::string &name, bool overlap, bool loop);
    void PlaySFXStartLoop(const std::string &start, const std::string &loop); // counts as 'loop' for IsSamplePlaying/StopSample
    
    bool IsSamplePlaying(const std::string &name);
    void StopSample(const std::string &name);
    
    void PlayMusic(const std::string &name, int crossfadeMs = 500);
    void FadeMusic(int ms);
}
