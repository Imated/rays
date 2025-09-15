#include "Model.h"

#include <cfloat>
#include <string>

#include "assimp/cimport.h"
#include "assimp/Importer.hpp"
#include "assimp/postprocess.h"
#include "assimp/scene.h"
#include "assimp/Vertex.h"
#include "glm/common.hpp"
#include "glm/vec2.hpp"
#include "misc/Logger.h"

namespace raytracer {
    Model::Model(const char *filename): meshInfo() {
        Assimp::Importer importer;

        const unsigned flags =
            aiProcess_Triangulate |
            aiProcess_GenNormals |
            aiProcess_JoinIdenticalVertices |
            aiProcess_ImproveCacheLocality;


        const aiScene *scene = importer.ReadFile(filename, flags);
        if (!scene) {
            ERR("Failed to load %s!: %s", filename, aiGetErrorString());
            return;
        }

        const aiMesh* model = scene->mMeshes[0];

        vertices.clear();  vertices.reserve(model->mNumVertices);
        normals.clear();   normals.reserve(model->mNumVertices);

        for (unsigned i = 0; i < model->mNumVertices; ++i) {
            vertices[i] = vec3(model->mVertices[i].x, model->mVertices[i].y, model->mVertices[i].z);
            normals[i] = vec3(model->mNormals[i].x, model->mNormals[i].y, model->mNormals[i].z);
        }

        vec3 bbMin(FLT_MAX), bbMax(-FLT_MAX);
        for (auto& p : vertices) {
            bbMin = min(bbMin, p);
            bbMax = max(bbMax, p);
        }

        triangles.clear();
        triangles.reserve(model->mNumFaces);

        for (unsigned f = 0; f < model->mNumFaces; ++f) {
            const aiFace& face = model->mFaces[f];
            if (face.mNumIndices != 3)
                continue;

            const unsigned i0 = face.mIndices[0];
            const unsigned i1 = face.mIndices[1];
            const unsigned i2 = face.mIndices[2];

            vec3 p0 = vertices[i0], p1 = vertices[i1], p2 = vertices[i2];
            vec3 n0 = normals[i0],  n1 = normals[i1],  n2 = normals[i2];

            Triangle tri = {
                vec4(p0, 0.0f), vec4(p1, 0.0f), vec4(p2, 0.0f),
                vec4(n0, 0.0f), vec4(n1, 0.0f), vec4(n2, 0.0f)
            };
            triangles.push_back(tri);
        }

        meshInfo = {
            0u,
            static_cast<uint32_t>(triangles.size()),
            0, 0,
            vec4(bbMin, 0),
            vec4(bbMax, 0),
            vec4(1, 1, 1, 0),
            vec4(0, 0, 0, 0)
        };
    }

    Model::~Model() = default;

    void Model::addTriangles(std::vector<Triangle>& triangles) const {
        triangles.insert(triangles.end(), this->triangles.begin(), this->triangles.end());
    }

    void Model::addMesh(std::vector<MeshInfo>& meshes) const {
        meshes.push_back(meshInfo);
    }
}

