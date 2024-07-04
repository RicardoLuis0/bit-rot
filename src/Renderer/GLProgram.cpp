#include "Renderer/GLProgram.h"

#include "Common.h"
#include <map>
#include <stdexcept>

using namespace Renderer::Internal;

static std::string glShaderErrMsg(GLuint shader)
{
    std::string tmp;
    GLint len;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &len);
    if(len > 0)
    {
        tmp.resize(len - 1);
        glGetShaderInfoLog(shader, len, &len, tmp.data());
    }
    return tmp;
}

static std::string glProgramErrMsg(GLuint program)
{
    std::string tmp;
    GLint len;
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &len);
    if(len > 0)
    {
        tmp.resize(len - 1);
        glGetProgramInfoLog(program, len, &len, tmp.data());
    }
    return tmp;
}

static bool CompileShader(GLuint shader)
{
    glCompileShader(shader);
    GLint ok;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
    return ok;
}

static bool LinkProgram(GLuint program)
{
    glLinkProgram(program);
    GLint ok;
    glGetProgramiv(program, GL_LINK_STATUS, &ok);
    return ok;
}

static bool ValidateProgram(GLuint program)
{
    glValidateProgram(program);
    GLint ok;
    glGetProgramiv(program, GL_VALIDATE_STATUS, &ok);
    return ok;
}

static void LoadShader(GLuint shader, const std::string &source)
{
    const GLchar * src = reinterpret_cast<const GLchar*>(source.data());
    GLint len = source.size();
    glShaderSource(shader, 1, &src, &len);
}

namespace Renderer
{
    namespace Internal
    {
        std::map<std::string, GLuint> vertexShaderCache;
        std::map<std::string, GLuint> fragShaderCache;
    }
    
    void GLProgram::CompileAndLink(const std::string &programName, const std::string &vertexFilename, const std::string &fragmentFilename)
    {
        std::string vertexFilePath = "Fx/" + vertexFilename;
        std::string fragmentFilePath = "Fx/" + fragmentFilename;
        
        GLuint vertexShader = 0;
        GLuint fragShader = 0;
        
        if(program) glDeleteProgram(program);
        
        glCheckErrorsDebug();
        
        program = glCreateProgram();
        
        if(!program)
        {
            throw FatalError(glErrMsg());
        }
        
        LogDebug("GL Program Created for "+Util::QuoteString(programName));
        
        if(vertexShaderCache.contains(vertexFilePath))
        {
            vertexShader = vertexShaderCache[vertexFilePath];
            
            LogDebug("Vertex Shader Fetched from Cache");
        }
        else
        {
            std::string vertexShaderSource = Util::ReadFile(vertexFilePath);
            
            LogDebug("Vertex Shader Source Fetched from Disk ("+Util::QuoteString(vertexFilePath)+")");
            
            if(!(
                    vertexShader = glCreateShader(GL_VERTEX_SHADER)
            )) {
                throw FatalError(glErrMsg());
            }
            
            LogDebug("Vertex Shader Created");
            
            LoadShader(vertexShader, vertexShaderSource);
            
            glCheckErrors();
            
            LogDebug("Vertex Shader Loaded");
            
            if(!CompileShader(vertexShader))
            {
                throw FatalError("Failed to compile Vertex Shader "+Util::QuoteString(vertexFilePath)+":\n"+glShaderErrMsg(vertexShader));
            }
            
            LogDebug("Vertex Shader Compiled");
            
            vertexShaderCache[vertexFilePath] = vertexShader;
            
            LogDebug("Vertex Shader Added to Cache");
        }
        
        glAttachShader(program, vertexShader);
        
        glCheckErrors();
        
        LogDebug("Vertex Shader Attached");
        
        if(fragShaderCache.contains(fragmentFilePath))
        {
            fragShader = fragShaderCache[fragmentFilePath];
            
            LogDebug("Vertex Shader Fetched from Cache");
        }
        else
        {
            std::string fragShaderSource = Util::ReadFile(fragmentFilePath);
            
            LogDebug("Fragment Shader Fetched from Disk ("+Util::QuoteString(fragmentFilePath)+")");
            
            if(!(
                    fragShader = glCreateShader(GL_FRAGMENT_SHADER)
            )) {
                throw FatalError(glErrMsg());
            }
            
            LogDebug("Fragment Shader Created");
            
            LoadShader(fragShader, fragShaderSource);
            
            glCheckErrors();
            
            LogDebug("Fragment Shader Loaded");
            
            if(!CompileShader(fragShader))
            {
                throw FatalError("Failed to compile Fragment Shader "+Util::QuoteString(fragmentFilePath)+":\n"+glShaderErrMsg(fragShader));
            }
            
            LogDebug("Fragment Shader Compiled");
            
            fragShaderCache[fragmentFilePath] = fragShader;
            
            LogDebug("Fragment Shader Added to Cache");
        }
        
        glAttachShader(program, fragShader);
        
        glCheckErrors();
        
        LogDebug("Fragment Shader Attached");
        
        if(!LinkProgram(program))
        {
            throw FatalError("Failed to link Program "+Util::QuoteString(programName)+":\n"+glProgramErrMsg(program));
        }
        
        LogDebug("Linked Program "+Util::QuoteString(programName));
        
        if(!ValidateProgram(program))
        {
            throw FatalError("Failed to validate Program "+Util::QuoteString(programName)+":\n"+glProgramErrMsg(program));
        }
        
        LogDebug("Validated Program "+Util::QuoteString(programName));
    }
    
}
