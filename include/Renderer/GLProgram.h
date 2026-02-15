#pragma once
#include "Renderer/Internal.h"
#include <map>

namespace Renderer
{
    struct GLProgram
    {
        GLuint program = 0;
        
        std::map<std::string, int> uniform_locations;
        
        void CompileAndLink(const std::string &programName, const std::string &vertexFilename, const std::string &fragmentFilename);
        
        inline void use()
        {
            using namespace Renderer::Internal;
            glUseProgram(program); glCheckErrorsDebug();
        }
        
        inline void setInt(int index, int x)
        {
            using namespace Renderer::Internal;
            glProgramUniform1i(program, index, x);
            glCheckErrorsDebug([=, this](){return "glProgramUniform1i("+std::to_string(program)+", "+std::to_string(index)+", "+std::to_string(x)+")";});
        }
        
        inline void setInt(int index, int x, int y)
        {
            using namespace Renderer::Internal;
            glProgramUniform2i(program, index, x, y);
            glCheckErrorsDebug([=, this](){return "glProgramUniform2i("+std::to_string(program)+", "+std::to_string(index)+", "+std::to_string(x)+", "+std::to_string(y)+")";});
        }
        
        inline void setInt(int index, int x, int y, int z)
        {
            using namespace Renderer::Internal;
            glProgramUniform3i(program, index, x, y, z);
            glCheckErrorsDebug([=, this](){return "glProgramUniform3i("+std::to_string(program)+", "+std::to_string(index)+", "+std::to_string(x)+", "+std::to_string(y)+", "+std::to_string(z)+")";});
        }
        
        inline void setInt(int index, int x, int y, int z, int w)
        {
            using namespace Renderer::Internal;
            glProgramUniform4i(program, index, x, y, z, w);
            glCheckErrorsDebug([=, this](){return "glProgramUniform4i("+std::to_string(program)+", "+std::to_string(index)+", "+std::to_string(x)+", "+std::to_string(y)+", "+std::to_string(z)+", "+std::to_string(w)+")";});
        }
        
        inline void setUInt(int index, unsigned x)
        {
            using namespace Renderer::Internal;
            glProgramUniform1ui(program, index, x);
            glCheckErrorsDebug([=, this](){return "glProgramUniform1ui("+std::to_string(program)+", "+std::to_string(index)+", "+std::to_string(x)+")";});
        }
        
        inline void setUInt(int index, unsigned x, unsigned y)
        {
            using namespace Renderer::Internal;
            glProgramUniform2ui(program, index, x, y);
            glCheckErrorsDebug([=, this](){return "glProgramUniform2ui("+std::to_string(program)+", "+std::to_string(index)+", "+std::to_string(x)+", "+std::to_string(y)+")";});
        }
        
        inline void setUInt(int index, unsigned x, unsigned y, unsigned z)
        {
            using namespace Renderer::Internal;
            glProgramUniform3ui(program, index, x, y, z);
            glCheckErrorsDebug([=, this](){return "glProgramUniform3ui("+std::to_string(program)+", "+std::to_string(index)+", "+std::to_string(x)+", "+std::to_string(y)+", "+std::to_string(z)+")";});
        }
        
        inline void setUInt(int index, unsigned x, unsigned y, unsigned z, unsigned w)
        {
            using namespace Renderer::Internal;
            glProgramUniform4ui(program, index, x, y, z, w);
            glCheckErrorsDebug([=, this](){return "glProgramUniform4ui("+std::to_string(program)+", "+std::to_string(index)+", "+std::to_string(x)+", "+std::to_string(y)+", "+std::to_string(z)+", "+std::to_string(w)+")";});
        }
        
        inline void setFloat(int index, float x)
        {
            using namespace Renderer::Internal;
            glProgramUniform1f(program, index, x);
            glCheckErrorsDebug([=, this](){return "glProgramUniform1f("+std::to_string(program)+", "+std::to_string(index)+", "+std::to_string(x)+")";});
        }
        
        inline void setFloat(int index, float x, float y)
        {
            using namespace Renderer::Internal;
            glProgramUniform2f(program, index, x, y);
            glCheckErrorsDebug([=, this](){return "glProgramUniform2f("+std::to_string(program)+", "+std::to_string(index)+", "+std::to_string(x)+", "+std::to_string(y)+")";});
        }
        
        inline void setFloat(int index, float x, float y, float z)
        {
            using namespace Renderer::Internal;
            glProgramUniform3f(program, index, x, y, z);
            glCheckErrorsDebug([=, this](){return "glProgramUniform3f("+std::to_string(program)+", "+std::to_string(index)+", "+std::to_string(x)+", "+std::to_string(y)+", "+std::to_string(z)+")";});
        }
        
        inline void setFloat(int index, float x, float y, float z, float w)
        {
            using namespace Renderer::Internal;
            glProgramUniform4f(program, index, x, y, z, w);
            glCheckErrorsDebug([=, this](){return "glProgramUniform4f("+std::to_string(program)+", "+std::to_string(index)+", "+std::to_string(x)+", "+std::to_string(y)+", "+std::to_string(z)+", "+std::to_string(w)+")";});
        }
        
        // --------------------------------------
        // named
        // --------------------------------------
        
        inline void setInt(const std::string &name, int x)
        {
            using namespace Renderer::Internal;
            int index;
            if(auto it = uniform_locations.find(name); it == uniform_locations.end())
            {
                index = glGetUniformLocation(program, name.c_str());
                glCheckErrorsDebug([=, this](){return "glGetUniformLocation("+std::to_string(program)+", '"+name+"')";});
                uniform_locations.insert({name,index});
            }
            else
            {
                index = it->second;
            }
            
            glProgramUniform1i(program, index, x);
            glCheckErrorsDebug([=, this](){return "glProgramUniform1i("+std::to_string(program)+", '"+name+"' ("+std::to_string(index)+"), "+std::to_string(x)+")";});
        }
        
        inline void setInt(const std::string &name, int x, int y)
        {
            using namespace Renderer::Internal;
            int index;
            if(auto it = uniform_locations.find(name); it == uniform_locations.end())
            {
                index = glGetUniformLocation(program, name.c_str());
                glCheckErrorsDebug([=, this](){return "glGetUniformLocation("+std::to_string(program)+", '"+name+"')";});
                uniform_locations.insert({name,index});
            }
            else
            {
                index = it->second;
            }
            
            glProgramUniform2i(program, index, x, y);
            glCheckErrorsDebug([=, this](){return "glProgramUniform2i("+std::to_string(program)+", '"+name+"' ("+std::to_string(index)+"), "+std::to_string(x)+", "+std::to_string(y)+")";});
        }
        
        inline void setInt(const std::string &name, int x, int y, int z)
        {
            using namespace Renderer::Internal;
            int index;
            if(auto it = uniform_locations.find(name); it == uniform_locations.end())
            {
                index = glGetUniformLocation(program, name.c_str());
                glCheckErrorsDebug([=, this](){return "glGetUniformLocation("+std::to_string(program)+", '"+name+"')";});
                uniform_locations.insert({name,index});
            }
            else
            {
                index = it->second;
            }
            
            glProgramUniform3i(program, index, x, y, z);
            glCheckErrorsDebug([=, this](){return "glProgramUniform3i("+std::to_string(program)+", '"+name+"' ("+std::to_string(index)+"), "+std::to_string(x)+", "+std::to_string(y)+", "+std::to_string(z)+")";});
        }
        
        inline void setInt(const std::string &name, int x, int y, int z, int w)
        {
            using namespace Renderer::Internal;
            int index;
            if(auto it = uniform_locations.find(name); it == uniform_locations.end())
            {
                index = glGetUniformLocation(program, name.c_str());
                glCheckErrorsDebug([=, this](){return "glGetUniformLocation("+std::to_string(program)+", '"+name+"')";});
                uniform_locations.insert({name,index});
            }
            else
            {
                index = it->second;
            }
            
            glProgramUniform4i(program, index, x, y, z, w);
            glCheckErrorsDebug([=, this](){return "glProgramUniform4i("+std::to_string(program)+", '"+name+"' ("+std::to_string(index)+"), "+std::to_string(x)+", "+std::to_string(y)+", "+std::to_string(z)+", "+std::to_string(w)+")";});
        }
        
        inline void setUInt(const std::string &name, unsigned x)
        {
            using namespace Renderer::Internal;
            int index;
            if(auto it = uniform_locations.find(name); it == uniform_locations.end())
            {
                index = glGetUniformLocation(program, name.c_str());
                glCheckErrorsDebug([=, this](){return "glGetUniformLocation("+std::to_string(program)+", '"+name+"')";});
                uniform_locations.insert({name,index});
            }
            else
            {
                index = it->second;
            }
            
            glProgramUniform1ui(program, index, x);
            glCheckErrorsDebug([=, this](){return "glProgramUniform1ui("+std::to_string(program)+", '"+name+"' ("+std::to_string(index)+"), "+std::to_string(x)+")";});
        }
        
        inline void setUInt(const std::string &name, unsigned x, unsigned y)
        {
            using namespace Renderer::Internal;
            int index;
            if(auto it = uniform_locations.find(name); it == uniform_locations.end())
            {
                index = glGetUniformLocation(program, name.c_str());
                glCheckErrorsDebug([=, this](){return "glGetUniformLocation("+std::to_string(program)+", '"+name+"')";});
                uniform_locations.insert({name,index});
            }
            else
            {
                index = it->second;
            }
            
            glProgramUniform2ui(program, index, x, y);
            glCheckErrorsDebug([=, this](){return "glProgramUniform2ui("+std::to_string(program)+", '"+name+"' ("+std::to_string(index)+"), "+std::to_string(x)+", "+std::to_string(y)+")";});
        }
        
        inline void setUInt(const std::string &name, unsigned x, unsigned y, unsigned z)
        {
            using namespace Renderer::Internal;
            int index;
            if(auto it = uniform_locations.find(name); it == uniform_locations.end())
            {
                index = glGetUniformLocation(program, name.c_str());
                glCheckErrorsDebug([=, this](){return "glGetUniformLocation("+std::to_string(program)+", '"+name+"')";});
                uniform_locations.insert({name,index});
            }
            else
            {
                index = it->second;
            }
            
            glProgramUniform3ui(program, index, x, y, z);
            glCheckErrorsDebug([=, this](){return "glProgramUniform3ui("+std::to_string(program)+", '"+name+"' ("+std::to_string(index)+"), "+std::to_string(x)+", "+std::to_string(y)+", "+std::to_string(z)+")";});
        }
        
        inline void setUInt(const std::string &name, unsigned x, unsigned y, unsigned z, unsigned w)
        {
            using namespace Renderer::Internal;
            int index;
            if(auto it = uniform_locations.find(name); it == uniform_locations.end())
            {
                index = glGetUniformLocation(program, name.c_str());
                glCheckErrorsDebug([=, this](){return "glGetUniformLocation("+std::to_string(program)+", '"+name+"')";});
                uniform_locations.insert({name,index});
            }
            else
            {
                index = it->second;
            }
            
            glProgramUniform4ui(program, index, x, y, z, w);
            glCheckErrorsDebug([=, this](){return "glProgramUniform4ui("+std::to_string(program)+", '"+name+"' ("+std::to_string(index)+"), "+std::to_string(x)+", "+std::to_string(y)+", "+std::to_string(z)+", "+std::to_string(w)+")";});
        }
        
        inline void setFloat(const std::string &name, float x)
        {
            using namespace Renderer::Internal;
            int index;
            if(auto it = uniform_locations.find(name); it == uniform_locations.end())
            {
                index = glGetUniformLocation(program, name.c_str());
                glCheckErrorsDebug([=, this](){return "glGetUniformLocation("+std::to_string(program)+", '"+name+"')";});
                uniform_locations.insert({name,index});
            }
            else
            {
                index = it->second;
            }
            
            glProgramUniform1f(program, index, x);
            glCheckErrorsDebug([=, this](){return "glProgramUniform1f("+std::to_string(program)+", '"+name+"' ("+std::to_string(index)+"), "+std::to_string(x)+")";});
        }
        
        inline void setFloat(const std::string &name, float x, float y)
        {
            using namespace Renderer::Internal;
            int index;
            if(auto it = uniform_locations.find(name); it == uniform_locations.end())
            {
                index = glGetUniformLocation(program, name.c_str());
                glCheckErrorsDebug([=, this](){return "glGetUniformLocation("+std::to_string(program)+", '"+name+"')";});
                uniform_locations.insert({name,index});
            }
            else
            {
                index = it->second;
            }
            
            glProgramUniform2f(program, index, x, y);
            glCheckErrorsDebug([=, this](){return "glProgramUniform2f("+std::to_string(program)+", '"+name+"' ("+std::to_string(index)+"), "+std::to_string(x)+", "+std::to_string(y)+")";});
        }
        
        inline void setFloat(const std::string &name, float x, float y, float z)
        {
            using namespace Renderer::Internal;
            int index;
            if(auto it = uniform_locations.find(name); it == uniform_locations.end())
            {
                index = glGetUniformLocation(program, name.c_str());
                glCheckErrorsDebug([=, this](){return "glGetUniformLocation("+std::to_string(program)+", '"+name+"')";});
                uniform_locations.insert({name,index});
            }
            else
            {
                index = it->second;
            }
            
            glProgramUniform3f(program, index, x, y, z);
            glCheckErrorsDebug([=, this](){return "glProgramUniform3f("+std::to_string(program)+", '"+name+"' ("+std::to_string(index)+"), "+std::to_string(x)+", "+std::to_string(y)+", "+std::to_string(z)+")";});
        }
        
        inline void setFloat(const std::string &name, float x, float y, float z, float w)
        {
            using namespace Renderer::Internal;
            int index;
            if(auto it = uniform_locations.find(name); it == uniform_locations.end())
            {
                index = glGetUniformLocation(program, name.c_str());
                glCheckErrorsDebug([=, this](){return "glGetUniformLocation("+std::to_string(program)+", '"+name+"')";});
                uniform_locations.insert({name,index});
            }
            else
            {
                index = it->second;
            }
            
            glProgramUniform4f(program, index, x, y, z, w);
            glCheckErrorsDebug([=, this](){return "glProgramUniform4f("+std::to_string(program)+", '"+name+"' ("+std::to_string(index)+"), "+std::to_string(x)+", "+std::to_string(y)+", "+std::to_string(z)+", "+std::to_string(w)+")";});
        }
    };
} 
