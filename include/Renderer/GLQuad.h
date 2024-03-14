#pragma once

#include "Common.h"
#include "Renderer/Internal.h"
#include "Renderer/GLTexture.h"
#include "Renderer/GLProgram.h"
#include "Renderer/GLUniformBuffer.h"
#include "Renderer/GLFrameBuffer.h"

#include <vector>
    #include <span>

namespace Renderer
{
    struct GLQuad
    {
    private:
        inline void ActivateTexturesAndBuffers(unsigned programIndex)
        {
            size_t n1 = Textures.size();
            for(unsigned i = 0; i < n1; i++)
            {
                if(programIndex < Textures[i].size() && Textures[i][programIndex])
                {
                    glActiveTexture(GL_TEXTURE0 + i);
                    glBindTexture(GL_TEXTURE_2D, Textures[i][programIndex]->index);
                }
            }
            
            size_t n2 = UniformBuffers.size();
            for(unsigned i = 0; i < n2; i++)
            {
                if(programIndex < UniformBuffers[i].size() && UniformBuffers[i][programIndex])
                {
                    glBindBufferBase(GL_UNIFORM_BUFFER, i, UniformBuffers[i][programIndex]->index);
                    break;
                }
            }
        }
        
        inline void PreRender()
        {
            glBindVertexArray(VAO);
            glEnableVertexAttribArray(0);
            glEnableVertexAttribArray(1);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
        }
        
        inline void PostRender()
        {
            //unbind
            for(unsigned i = 0; i < Textures.size(); i++)
            {
                glActiveTexture(GL_TEXTURE0 + i);
                glBindTexture(GL_TEXTURE_2D, 0);
            }
            
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
            glDisableVertexAttribArray(0);
            glBindVertexArray(0);
            
            Internal::glCheckErrors();
        }
    public:
        GLuint VBO = 0;
        GLuint IBO = 0;
        GLuint VAO = 0;
        
        std::vector<GLProgram *> Programs;
        std::vector<std::vector<GLUniformBuffer *>> UniformBuffers;
        std::vector<std::vector<GLTexture *>> Textures;
        
        void Gen(
            float x1 = -1.0,
            float y1 = -1.0,
            float x2 =  1.0,
            float y2 =  1.0
        );
        
        void Gen(
            GLProgram *program,
            float x1 = -1.0,
            float y1 = -1.0,
            float x2 =  1.0,
            float y2 =  1.0
        )
        {
            Gen(x1, y1, x2, y2);
            Programs.push_back(program);
        }
        
        template<Util::ContainerOf<GLProgram*> T>
        void Gen(
            T programs,
            float x1 = -1.0,
            float y1 = -1.0,
            float x2 =  1.0,
            float y2 =  1.0
        )// requires std::same_as<std::remove_cvref_t<decltype(*std::begin(programs))>, GLProgram*>
        {
            Gen(x1, y1, x2, y2);
            Programs.insert(Programs.end(), std::begin(programs), std::end(programs));
        }
        
        inline void addUBO(GLUniformBuffer * ubo)
        {
            UniformBuffers.push_back({ubo});
        }
        
        template<Util::ContainerOf<GLUniformBuffer*> T>
        inline void addUBO(T ubo)
        {
            UniformBuffers.emplace_back(std::begin(ubo), std::end(ubo));
        }
        
        inline void addTexture(GLTexture * tex)
        {
            Textures.push_back({tex});
        }
        
        template<Util::ContainerOf<GLTexture*> T>
        inline void addTexture(T tex)
        {
            Textures.emplace_back(std::begin(tex), std::end(tex));
        }
        
        inline void setTexture(size_t i, GLTexture * tex)
        {
            Textures[i] = {tex};
        }
        
        template<Util::ContainerOf<GLTexture*> T>
        inline void setTexture(size_t i, T tex)
        {
            Textures[i] = {std::begin(tex), std::end(tex)};
        }
        
        inline void addProgram(GLProgram * program)
        {
            Programs.push_back(program);
        }
        
        
        inline void Render()
        {
            PreRender();
            
            size_t n = Programs.size();
            for(size_t i = 0; i < n; i++)
            {
                ActivateTexturesAndBuffers(i);
                Programs[i]->use();
                //draw
                glDrawElements(GL_TRIANGLE_FAN, 4, GL_UNSIGNED_INT, NULL);
            }
            
            PostRender();
        }
        
        template<Util::ContainerOf<GLFrameBuffer*> T>
        inline void Render(T buffers)
        {
            PreRender();
            
            size_t n1 = Programs.size();
            size_t n2 = buffers.size();
            for(size_t i = 0; i < n1; i++)
            {
                if(i < n2)
                {
                    GLFrameBuffer* buf = buffers[i];
                    if(buf)
                    {
                        buf->Use();
                    }
                    else
                    {
                        Internal::ResetFrameBuffer();
                    }
                }
                glClear(GL_COLOR_BUFFER_BIT);
                ActivateTexturesAndBuffers(i);
                Programs[i]->use();
                //draw
                glDrawElements(GL_TRIANGLE_FAN, 4, GL_UNSIGNED_INT, NULL);
            }
            
            PostRender();
        }
    };
}
