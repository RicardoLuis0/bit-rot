#pragma once

#include "Renderer.h"
#include "Log.h"
#include <SDL2/SDL.h>
#include <cstdint>
#include <GL/glew.h>
#include <SDL2/SDL_opengl.h>
#include <GL/glu.h>
#include <stdexcept>
#include <map>

namespace Renderer::Internal
{
    extern uint32_t window_width;
    extern uint32_t window_height;
    
    extern std::map<std::string, GLuint> vertexShaderCache;
    extern std::map<std::string, GLuint> fragShaderCache;
    
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
    
    inline void glCheckErrorsInternal(std::string msgPre)
    {
        if(GLenum err = glGetError(); err != GL_NO_ERROR)
        {
            std::string msg = glErrMsg(err);
            while((err = glGetError()) != GL_NO_ERROR)
            {
                msg += "\n" + glErrMsg(err);
            }
            throw FatalError(msgPre + msg, 2);
        }
    }
    
    inline void glCheckErrorsLine(std::string_view fn_namespace, std::string_view fn_name, std::string_view file_name, int line)
    {
        std::string msg = "";
        if(fn_namespace.size() > 0)
        {
            assert(fn_name.size() > 0);
            msg += "[" + std::string(fn_namespace) + "::" + std::string(fn_name) + "] ";
        }
        else if(fn_name.size() > 0)
        {
            msg += "[" + std::string(fn_name) + "] ";
        }
        
        if(file_name.size() > 0)
        {
            msg += "[" + std::string(file_name) + ":" + std::to_string(line) + "] ";
        }
        glCheckErrorsInternal(msg);
    }
    
    
    #ifdef _MSC_VER
        #define glCheckErrors()\
            glCheckErrorsLine(std::string_view(__FUNCSIG__ + namespace_start(__FUNCSIG__) , namespace_len(__FUNCSIG__)), std::string_view(__FUNCSIG__ + funcname_start(__FUNCSIG__) , funcname_len(__FUNCSIG__)), std::string_view(__FILE__ + filename_start(__FILE__)) , __LINE__)
    #elif defined(__GNUC__)
        #define glCheckErrors()\
            glCheckErrorsLine(std::string_view(__PRETTY_FUNCTION__ + namespace_start(__PRETTY_FUNCTION__) , namespace_len(__PRETTY_FUNCTION__)), std::string_view(__PRETTY_FUNCTION__ + funcname_start(__PRETTY_FUNCTION__) , funcname_len(__PRETTY_FUNCTION__)), std::string_view(__FILE__ + filename_start(__FILE__)) , __LINE__)
    #else
        #define glCheckErrors()\
            glCheckErrorsLine({}, std::string_view(__func__), std::string_view(__FILE__ + filename_start(__FILE__)) , __LINE__)
    #endif
    
    #ifdef NDEBUG
        #define glCheckErrorsDebug()
    #else
        #define glCheckErrorsDebug glCheckErrors
    #endif
    
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
