#pragma once
#include "Renderer/Internal.h"

namespace Renderer
{
    
    struct GLTexture
    {
    private:
        uint32_t width;
        uint32_t height;
    public:
        GLuint index = 0;
        
        void InitNew(uint32_t width, uint32_t height);
        void UpdateRGBA8(const uint32_t * data, uint32_t width, uint32_t height);
        
        void LoadRGBA8(const uint32_t * data, uint32_t width, uint32_t height);
    };
}
