#pragma once
#include "Renderer/Internal.h"


namespace Renderer
{
    struct GLProgram
    {
        GLuint program = 0;
        
        void CompileAndLink(const std::string &programName, const std::string &vertexFilename, const std::string &fragmentFilename);
        
        inline void use()
        {
            glUseProgram(program);
        }
        
        inline void setInt(int index, int x)
        {
            glProgramUniform1i(program, index, x);
        }
        
        inline void setInt(int index, int x, int y)
        {
            glProgramUniform2i(program, index, x, y);
        }
        
        inline void setInt(int index, int x, int y, int z)
        {
            glProgramUniform3i(program, index, x, y, z);
        }
        
        inline void setInt(int index, int x, int y, int z, int w)
        {
            glProgramUniform4i(program, index, x, y, z, w);
        }
        
        inline void setUInt(int index, unsigned x)
        {
            glProgramUniform1ui(program, index, x);
        }
        
        inline void setUInt(int index, unsigned x, unsigned y)
        {
            glProgramUniform2ui(program, index, x, y);
        }
        
        inline void setUInt(int index, unsigned x, unsigned y, unsigned z)
        {
            glProgramUniform3ui(program, index, x, y, z);
        }
        
        inline void setUInt(int index, unsigned x, unsigned y, unsigned z, unsigned w)
        {
            glProgramUniform4ui(program, index, x, y, z, w);
        }
        
        inline void setFloat(int index, float x)
        {
            glProgramUniform1f(program, index, x);
        }
        
        inline void setFloat(int index, float x, float y)
        {
            glProgramUniform2f(program, index, x, y);
        }
        
        inline void setFloat(int index, float x, float y, float z)
        {
            glProgramUniform3f(program, index, x, y, z);
        }
        
        inline void setFloat(int index, float x, float y, float z, float w)
        {
            glProgramUniform4f(program, index, x, y, z, w);
        }
    };
} 
