#include "Renderer/GLFrameBuffer.h"

#include "Common.h"
#include <stdexcept>

using namespace Renderer::Internal;

namespace Renderer
{
    void GLFrameBuffer::Init(uint32_t _width, uint32_t _height)
    {
        glGenFramebuffers(1, &index);
        
        glCheckErrorsDebug();
        
        ResizeInternal(_width, _height);
    }
    
    void GLFrameBuffer::Resize(uint32_t _width, uint32_t _height)
    {
        glDeleteTextures(1, &colorTexture.index);
        
        glCheckErrorsDebug();
        
        colorTexture.index = 0;
        
        ResizeInternal(_width, _height);
    }
    
    void GLFrameBuffer::ResizeInternal(uint32_t _width, uint32_t _height)
    {
        width = _width;
        height = _height;
        colorTexture.InitNew(width, height);
        
        glBindFramebuffer(GL_FRAMEBUFFER, index);
        
        glCheckErrorsDebug();
        
        glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTexture.index, 0);
        
        glCheckErrorsDebug();
        
        if(GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER); status != GL_FRAMEBUFFER_COMPLETE)
        {
            throw FatalError("Failed to create/resize framebuffer, status: "+std::to_string(status));
        }
        
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        
        glCheckErrorsDebug();
    }
}
