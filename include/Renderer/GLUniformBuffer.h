#pragma once
#include "Renderer/Internal.h"

namespace Renderer
{
    struct GLUniformBuffer
    {
        GLuint index = 0;
        
        void Update(void *data, size_t offset, size_t len);
        
        void Init(void *data, size_t len);
        
        template<typename T>
        void Update(T *data, size_t vlaLen = 0)
        {
            Update((void*)data, 0, sizeof(T) + vlaLen);
        }
        
        template<typename T>
        void Init(T *data, size_t vlaLen = 0)
        {
            Init((void*)data, sizeof(T) + vlaLen);
        }
    };
}
