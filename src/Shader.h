#pragma once
#include <unordered_map>
#include "glad/glad.h"
#include "glm/vec2.hpp"

namespace raytracer {
    class Shader
    {
    public:
        GLuint shaderID;
        GLuint computeShaderID;
        Shader(const char* vertexFilename, const char* fragmentFilename);
        Shader(const char* vertexFilename, const char* fragmentFilename, const char* computeFilename);
        Shader(): shaderID(0) { }

        void setMatrix4x4(const char* name, const float* matrix, bool compute = false);
        void setMatrix3x3(const char* name, const float* matrix, bool compute = false);
        void setVector4(const char* name, GLfloat x, GLfloat y, GLfloat z, GLfloat w, bool compute = false);
        void setVector3(const char* name, GLfloat x, GLfloat y, GLfloat z, bool compute = false);
        void setVector2(const char* name, GLfloat x, GLfloat y, bool compute = false);
        void setUIVector2(const char* name, GLuint x, GLuint y, bool compute = false);
        void setFloat(const char* name, float value, bool compute = false);
        void setInt(const char* name, int value, bool compute = false);
        void setUInt(const char* name, int value, bool compute = false);
        void setBool(const char* name, bool value, bool compute = false);
        void setUIntArray(const char *name, GLsizei count, uint32_t* value, bool compute = false);
        void setUVec2Array(const char *name, GLsizei count, glm::uvec2* value, bool compute = false);

        void use() const;
        void useCompute() const;
        void del() const;
    private:
        int getUniformLocation(const char* name);
        int getComputeUniformLocation(const char* name);
        GLuint createShader(const char* filename, GLenum type) const;

        std::unordered_map<const char*, int> cachedUniformLocations;
        std::unordered_map<const char*, int> cachedComputeUniformLocations;
    };
}
