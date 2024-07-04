#include "Renderer/GLTexture.h"

#include "Common.h"
#include <stdexcept>

using namespace Renderer::Internal;

namespace Renderer
{
    static GLfloat blackBorder[] = { 0.0f, 0.0f, 0.0f, 0.0f };
    
    void GLTexture::InitNew(uint32_t new_width, uint32_t new_height)
    {
        if(index != 0) glDeleteTextures(1, &index);
        
        glCheckErrorsDebug();
        
        width = new_width;
        height = new_height;
        
        glCreateTextures(GL_TEXTURE_2D, 1, &index); glCheckErrorsDebug();
        
        glTextureStorage2D(index, 1, GL_RGBA8, width, height);
        
        glCheckErrors();
        
        glTextureParameteri(index, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER); glCheckErrorsDebug();
        glTextureParameteri(index, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER); glCheckErrorsDebug();
        glTextureParameterfv(index, GL_TEXTURE_BORDER_COLOR, blackBorder); glCheckErrorsDebug();
        glTextureParameteri(index, GL_TEXTURE_MIN_FILTER, GL_NEAREST); glCheckErrorsDebug();
        glTextureParameteri(index, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        
        glCheckErrors();
    }
    
    void GLTexture::UpdateRGBA8(const uint32_t * data, uint32_t new_width, uint32_t new_height)
    {
        if(index == 0 || width != new_width || height != new_height)
        {
            LoadRGBA8(data, new_width, new_height);
        }
        else
        {
            glTextureSubImage2D(index, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, (const void*)data);
            glCheckErrors();
        }
    }
    
    void GLTexture::LoadRGBA8(const uint32_t * data, uint32_t new_width, uint32_t new_height)
    {
        if(index != 0) glDeleteTextures(1, &index);
        
        width = new_width;
        height = new_height;
        
        glCreateTextures(GL_TEXTURE_2D, 1, &index);
        glTextureStorage2D(index, 1, GL_RGBA8, width, height);
        
        glCheckErrors();
        
        glTextureSubImage2D(index, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, (const void*)data);
        
        glTextureParameteri(index, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTextureParameteri(index, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        glTextureParameterfv(index, GL_TEXTURE_BORDER_COLOR, blackBorder);
        glTextureParameteri(index, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTextureParameteri(index, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        
        glCheckErrors();
    }
}
