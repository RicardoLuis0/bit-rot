#include "Renderer/GLUniformBuffer.h"

#include "Common.h"
#include <stdexcept>

using namespace Renderer::Internal;

namespace Renderer
{
    void GLUniformBuffer::Update(void *data, size_t offset, size_t len)
    {
        if(!index) return;
        glNamedBufferSubData(index, offset, len, data);
        
        glCheckErrors();
    }
    
    void GLUniformBuffer::Init(void *data, size_t len)
    {
        glCreateBuffers(1, &index);
        glNamedBufferStorage(index, len, data, GL_DYNAMIC_STORAGE_BIT | GL_MAP_READ_BIT | GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT);
        //glNamedBufferData(index, len, data, GL_DYNAMIC_DRAW);
        
        glCheckErrors();
    }
}
