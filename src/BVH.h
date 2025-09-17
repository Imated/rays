#pragma once
#include <vector>
#include "glm/glm.hpp"

using namespace glm;

namespace raytracer {
    struct BVHNode;

    class BVH {
    public:
        BVH(std::vector<vec3>& vertices, const std::vector<unsigned int>& indices);

        const std::vector<BVHNode>& getNodes()      const { return nodes; }
        const std::vector<uint32_t>& getTriIndices() const { return triangleIndexes; }
    private:
        static bool triCenterMinMax(const std::vector<vec3>& v,
                                           const std::vector<uint32_t>& idx,
                                           uint32_t triId,
                                           vec3& center, vec3& min, vec3& max)
        {
            if (const uint64_t triCount = idx.size() / 3ull; triId >= triCount)
                return false;

            const uint32_t aI = idx[3 * triId + 0];
            const uint32_t bI = idx[3 * triId + 1];
            const uint32_t cI = idx[3 * triId + 2];

            if (aI >= v.size() || bI >= v.size() || cI >= v.size())
                return false;

            const vec3 a = v[aI];
            const vec3 b = v[bI];
            const vec3 c = v[cI];

            min = glm::min(a, glm::min(b, c));
            max = glm::max(a, glm::max(b, c));
            center = (a + b + c) * (1.0f / 3.0f);
            return true;
        }

        void updateNodeBounds(uint32_t nodeIndex);
        void split(uint32_t nodeIdx, int depth = 0);

        const std::vector<vec3>& verticies;
        const std::vector<uint32_t>& indices;
        std::vector<uint32_t> triangleIndexes;
        std::vector<BVHNode> nodes;
        int nodesUsed;
    };

    struct BVHNode {
        vec4 min;
        vec4 max;
        uvec4 triIndex_triCount_childIndex;
    };
}
