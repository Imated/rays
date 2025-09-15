#pragma once
#include <vector>

#include "glm/vec3.hpp"
#include "glm/vec4.hpp"
struct aiFace;
using namespace glm;

namespace raytracer {
    struct Sphere {
        vec4 pos_radius;
        vec4 color_smoothness;
        vec4 emissiveColor_strength;
    };

    struct Triangle {
        vec4 posA;
        vec4 posB;
        vec4 posC;
        vec4 normalA;
        vec4 normalB;
        vec4 normalC;
    };

    struct MeshInfo {
        uint32_t firstTriangleIndex;
        uint32_t numTriangles;
        uint32_t _pad0;
        uint32_t _pad1;
        vec4 boundingBoxMin;
        vec4 boundingBoxMax;
        vec4 color_smoothness;
        vec4 emissiveColor_strength;
    };

    class Model {
    public:
        explicit Model(const char* filename);
        ~Model();

        void addTriangles(std::vector<Triangle>& triangles) const;
        void addMesh(std::vector<MeshInfo>& meshes) const;
    private:
        MeshInfo meshInfo;
        std::vector<vec3> vertices;
        std::vector<vec3> normals;
        std::vector<Triangle> triangles;
    };
}
