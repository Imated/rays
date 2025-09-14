#include "Shader.h"

#include <math.h>
#include <vector>

#include "glm/gtc/type_ptr.inl"
#include "misc/Utility.h"

namespace raytracer {
    Shader::Shader(const char* vertexFilename, const char* fragmentFilename)
    {
        shaderID = glCreateProgram();
        const auto vertexShader = createShader(vertexFilename, GL_VERTEX_SHADER);
        const auto fragmentShader = createShader(fragmentFilename, GL_FRAGMENT_SHADER);

        glAttachShader(shaderID, vertexShader);
        glAttachShader(shaderID, fragmentShader);
        glLinkProgram(shaderID);

        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
    }

    Shader::Shader(const char* vertexFilename, const char* fragmentFilename, const char* computeFilename)
    {
        shaderID = glCreateProgram();
        computeShaderID = glCreateProgram();
        const auto vertexShader = createShader(vertexFilename, GL_VERTEX_SHADER);
        const auto fragmentShader = createShader(fragmentFilename, GL_FRAGMENT_SHADER);
        const auto computeShader = createShader(computeFilename, GL_COMPUTE_SHADER);

        glAttachShader(shaderID, vertexShader);
        glAttachShader(shaderID, fragmentShader);
        glAttachShader(computeShaderID, computeShader);
        glLinkProgram(shaderID);
        glLinkProgram(computeShaderID);

        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        glDeleteShader(computeShader);
    }

    GLuint Shader::createShader(const char* filename, GLenum type) const
    {
        const std::string source = Utils::readFile(filename);
        const char* shaderSource = source.c_str();

        GLint isCompiled = 0;
        const GLuint shader = glCreateShader(type);
        glShaderSource(shader, 1, &shaderSource, nullptr);
        glCompileShader(shader);
        glGetShaderiv(shader, GL_COMPILE_STATUS, &isCompiled);
        if(!isCompiled)
        {
            GLint maxLength = 0;
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);
            std::vector<GLchar> errorLog(maxLength);
            glGetShaderInfoLog(shader, maxLength, &maxLength, errorLog.data());
            ERR(errorLog.data());
            ERR("Could not compile shader! ");
            throw std::runtime_error("Could not compile shader");
        }
        return shader;
    }

    #pragma region Shader Attributes
    int Shader::getUniformLocation(const char* name)
    {
        int location = 0;
        if (cachedUniformLocations.contains(name))
            location = cachedUniformLocations.at(name);
        else {
            location = glGetUniformLocation(shaderID, name);
            cachedUniformLocations[name] = location;
        }
        if (location == -1)
        {
            std::string msg = "Could not find uniform variable with name of '" + std::string(name) + "'!";
            const char* warningLog = msg.c_str();
            WARN(warningLog);
        }
        return location;
    }

    int Shader::getComputeUniformLocation(const char *name) {
        int location = 0;
        if (cachedComputeUniformLocations.contains(name))
            location = cachedComputeUniformLocations.at(name);
        else {
            location = glGetUniformLocation(computeShaderID, name);
            cachedComputeUniformLocations[name] = location;
        }
        if (location == -1)
        {
            std::string msg = "Could not find uniform variable with name of '" + std::string(name) + "'!";
            const char* warningLog = msg.c_str();
            WARN(warningLog);
        }
        return location;
    }

    void Shader::setMatrix4x4(const char* name, const float* matrix, bool compute)
    {
        int location = 0;
        if (compute)
            location = getComputeUniformLocation(name);
        else
            location = getUniformLocation(name);
        glUniformMatrix4fv(location, 1, GL_FALSE, matrix);
    }

    void Shader::setMatrix3x3(const char *name, const float *matrix, bool compute)
    {
        int location = 0;
        if (compute)
            location = getComputeUniformLocation(name);
        else
            location = getUniformLocation(name);
        glUniformMatrix3fv(location, 1, GL_FALSE, matrix);
    }

    void Shader::setVector4(const char* name, GLfloat x, GLfloat y, GLfloat z, GLfloat w, bool compute)
    {
        int location = 0;
        if (compute)
            location = getComputeUniformLocation(name);
        else
            location = getUniformLocation(name);
        glUniform4f(location, x, y, z, w);
    }

    void Shader::setVector3(const char* name, GLfloat x, GLfloat y, GLfloat z, bool compute)
    {
        int location = 0;
        if (compute)
            location = getComputeUniformLocation(name);
        else
            location = getUniformLocation(name);
        glUniform3f(location, x, y, z);
    }

    void Shader::setVector2(const char* name, GLfloat x, GLfloat y, bool compute)
    {
        int location = 0;
        if (compute)
            location = getComputeUniformLocation(name);
        else
            location = getUniformLocation(name);
        glUniform2f(location, x, y);
    }

    void Shader::setUIVector2(const char *name, GLuint x, GLuint y, bool compute)
    {
        int location = 0;
        if (compute)
            location = getComputeUniformLocation(name);
        else
            location = getUniformLocation(name);
        glUniform2ui(location, x, y);
    }

    void Shader::setFloat(const char* name, float value, bool compute)
    {
        int location = 0;
        if (compute)
            location = getComputeUniformLocation(name);
        else
            location = getUniformLocation(name);
        glUniform1f(location, value);
    }

    void Shader::setInt(const char *name, int value, bool compute)
    {
        int location = 0;
        if (compute)
            location = getComputeUniformLocation(name);
        else
            location = getUniformLocation(name);
        glUniform1i(location, value);
    }

    void Shader::setUInt(const char *name, int value, bool compute)
    {
        int location = 0;
        if (compute)
            location = getComputeUniformLocation(name);
        else
            location = getUniformLocation(name);
        glUniform1ui(location, value);
    }

    void Shader::setBool(const char* name, bool value, bool compute)
    {
        int location = 0;
        if (compute)
            location = getComputeUniformLocation(name);
        else
            location = getUniformLocation(name);
        glUniform1i(location, value);
    }

    void Shader::setUIntArray(const char *name, GLsizei count, uint32_t *value, bool compute)
    {
        int location = 0;
        if (compute)
            location = getComputeUniformLocation(name);
        else
            location = getUniformLocation(name);
        glUniform1uiv(location, count, value);
    }

    void Shader::setUVec2Array(const char *name, GLsizei count, glm::uvec2 *value, bool compute)
    {
        int location = 0;
        if (compute)
            location = getComputeUniformLocation(name);
        else
            location = getUniformLocation(name);
        glUniform2uiv(location, count, glm::value_ptr(value[0]));
    }


#pragma endregion

    void Shader::use() const
    {
        glUseProgram(shaderID);
    }

    void Shader::useCompute() const {
        glUseProgram(computeShaderID);
    }

    void Shader::del() const
    {
        glDeleteProgram(shaderID);
    }
}