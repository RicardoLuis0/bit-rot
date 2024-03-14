#include "Renderer/GLQuad.h"

#include "Common.h"
#include <stdexcept>

using namespace Renderer::Internal;

namespace Renderer
{
    void GLQuad::Gen(float x1, float y1, float x2, float y2)
    {
        GLfloat verts[] =
        {
            x1, y1,
            0.0, 0.0,
            x2, y1,
            1.0, 0.0,
            x2, y2,
            1.0, 1.0,
            x1, y2,
            0.0, 1.0
        };
        
        GLuint indices[] = { 0, 1, 2, 3 };
        
        glGenBuffers(2, &VBO);
        
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);
        
            glGenVertexArrays(1, &VAO);
            glBindVertexArray(VAO);
            glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, (sizeof(GLfloat) * 4), nullptr);
            glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, (sizeof(GLfloat) * 4), (void*)(sizeof(GLfloat) * 2) );
            glBindVertexArray(0);
        
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        
        glCheckErrors();
    }
}
