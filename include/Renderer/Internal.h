#pragma once

#include "Renderer.h"
#include <SDL2/SDL.h>
#include <cstdint>
#include <gl/glew.h>
#include <SDL2/SDL_opengl.h>
#include <gl/glu.h>
#include <stdexcept>

namespace Renderer::Internal
{
    extern uint32_t window_width;
    extern uint32_t window_height;
    
    inline std::string glErrMsg(GLenum err)
    {
        if(err == GL_INVALID_OPERATION || err == GL_INVALID_ENUM)
        {
            return std::string(reinterpret_cast<const char *>(gluErrorString(err))) + "\n\nYou might be using an incompatible version of RTSS, either update to at least version 7.3.5, or disable the overlay";
        }
        else
        {
            return reinterpret_cast<const char *>(gluErrorString(err));
        }
    }
    
    inline std::string glErrMsg()
    {
        return glErrMsg(glGetError());
    }
    
    inline void glCheckErrors()
    {
        if(GLenum err = glGetError(); err != GL_NO_ERROR)
        {
            std::string msg = glErrMsg(err);
            while((err = glGetError()) != GL_NO_ERROR)
            {
                msg += "\n" + glErrMsg(err);
            }
            throw std::runtime_error(msg);
        }
    }
    
    constexpr double ratio = 4.0/3.0;
    constexpr double ratio_tolerance = 0.1;
    
    inline void calcWidthHeight(uint32_t &width, uint32_t &height, uint32_t * x = nullptr, uint32_t * y = nullptr)
    {
        if(x) *x = 0;
        if(y) *y = 0;
        width = window_width;
        height = window_height;
        double windowRatio = double(window_width) / double(window_height);
        
        if(abs(windowRatio - ratio) > ratio_tolerance)
        {
            if(windowRatio > ratio)
            {   // pillarbox
                width = (window_height * 4) / 3;
                if(x) *x = (window_width - width) / 2;
            }
            else
            {   // lettebox
                height = (window_width * 3) / 4;
                if(y) *y = (window_height - height) / 2;
            }
        }
        
    }
    
    inline void ResetFrameBuffer()
    {
        uint32_t viewport_x = 0;
        uint32_t viewport_y = 0;
        uint32_t viewport_width = window_width;
        uint32_t viewport_height = window_height;
        
        calcWidthHeight(viewport_width, viewport_height, &viewport_x, &viewport_y);
        
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(viewport_x, viewport_y, viewport_width, viewport_height);
    }
}
