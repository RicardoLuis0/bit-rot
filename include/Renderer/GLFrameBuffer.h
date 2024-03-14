#pragma once
#include "Renderer/Internal.h"
#include "Renderer/GLTexture.h"

namespace Renderer
{
    struct GLFrameBuffer
    {
    private:
        void ResizeInternal(uint32_t width, uint32_t height);
    public:
        GLuint index = 0;
        uint32_t width, height;
        GLTexture colorTexture;
        //GLuint colorIndex;
        
        void Init(uint32_t width, uint32_t height);
        
        void Use()
        {
            glBindFramebuffer(GL_FRAMEBUFFER, index);
            glViewport(0, 0, width, height);
        }
        
        void Resize(uint32_t width, uint32_t height);
    };
}
