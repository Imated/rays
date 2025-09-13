﻿#pragma once
#include <unordered_map>
#include "glad/glad.h"
#include "glm/vec2.hpp"

namespace raytracer {
    class Shader
    {
    public:
        GLuint shaderID;
        Shader(const char* vertexFilename, const char* fragmentFilename);
        Shader(): shaderID(0) { }

        void setMatrix4x4(const char* name, const float* matrix);
        void setVector4(const char* name, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
        void setVector3(const char* name, GLfloat x, GLfloat y, GLfloat z);
        void setVector2(const char* name, GLfloat x, GLfloat y);
        void setUIVector2(const char* name, GLuint x, GLuint y);
        void setFloat(const char* name, float value);
        void setInt(const char* name, int value);
        void setUInt(const char* name, int value);
        void setBool(const char* name, bool value);
        void setUIntArray(const char *name, GLsizei count, uint32_t* value);
        void setUVec2Array(const char *name, GLsizei count, glm::uvec2* value);

        void use() const;
        void del() const;
    private:
        int getUniformLocation(const char* name);
        GLuint createShader(const char* filename, GLenum type) const;

        std::unordered_map<const char*, int> cachedUniformLocations;
    };
}
