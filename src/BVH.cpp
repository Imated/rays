#include "BVH.h"

#include <cstdio>
#include <cfloat>

namespace raytracer {
    BVH::BVH(std::vector<vec3>& vertices, const std::vector<unsigned int>& indices): verticies(vertices), indices(indices)  {
        const auto triCount = static_cast<uint32_t>(indices.size() / 3);
        triangleIndexes.resize(triCount);

        for (uint32_t t = 0; t < triCount; ++t) {
            triangleIndexes[t] = t;
        }

        assert(!verticies.empty());
        assert(indices.size() % 3 == 0);
        assert(triangleIndexes.size() == indices.size()/3);

        nodes.reserve(triCount * 2);
        nodes.emplace_back();

        auto& root = nodes[0];
        root.triIndex_triCount_childIndex = uvec4(0, triCount, -1.0f, 0.f);
        updateNodeBounds(0);
        split(0, 0);
    }

    void BVH::updateNodeBounds(uint32_t nodeIndex) {
        const auto first = nodes[nodeIndex].triIndex_triCount_childIndex.x;
        const auto count = nodes[nodeIndex].triIndex_triCount_childIndex.y;

        vec3 min( FLT_MAX), max(-FLT_MAX);
        for (uint32_t i = 0; i < count; ++i) {
            const uint32_t triId = triangleIndexes[first + i];
            vec3 c, tmin, tmax;
            if (!triCenterMinMax(verticies, indices, triId, c, tmin, tmax)) {
                fprintf(stderr, "bad triId=%u first=%u count=%u idx=%zu v=%zu\n", triId, first, count, indices.size(), verticies.size());
                continue;
            }
            min = glm::min(min, tmin);
            max = glm::max(max, tmax);
        }
        nodes[nodeIndex].min = vec4(min, 0.0f);
        nodes[nodeIndex].max = vec4(max, 0.0f);
    }

    void BVH::split(uint32_t nodeIdx, int depth) {
        constexpr int maxDepth = 32;
        if (depth > maxDepth)
            return;

        auto first = nodes[nodeIdx].triIndex_triCount_childIndex.x;
        auto count = nodes[nodeIdx].triIndex_triCount_childIndex.y;

        if (count < 2)
            return;

        assert(first + count <= triangleIndexes.size());

        vec3 extent = vec3(nodes[nodeIdx].max) - vec3(nodes[nodeIdx].min);
        int axis = 0;
        if (extent.y > extent.z)
            axis = 1;
        if (extent.z > extent[axis])
            axis = 2;
        float splitPos = nodes[nodeIdx].min[axis] + extent[axis] * 0.5f;

        uint32_t i = first, j = first + count - 1;
        while (i <= j) {
            vec3 c, min, max;
            triCenterMinMax(verticies, indices, triangleIndexes[i], c, min, max);
            if (c[axis] < splitPos)
                ++i;
            else {
                std::swap(triangleIndexes[i], triangleIndexes[j]);
                if (j==0)
                    break;
                --j;
            }
        }

        uint32_t leftCount = i - first;
        if (leftCount == 0 || leftCount == count) {
            leftCount = count / 2;
            if (leftCount == 0)
                leftCount = 1;
            if (leftCount == count)
                leftCount = count - 1;
        }
        const uint32_t rightCount = count - leftCount;
        const uint32_t leftFirst  = first;
        const uint32_t rightFirst = first + leftCount;

        const auto leftIndex  = static_cast<uint32_t>(nodes.size());
        nodes.emplace_back();
        const auto rightIndex = static_cast<uint32_t>(nodes.size());
        nodes.emplace_back();

        nodes[nodeIdx].triIndex_triCount_childIndex = uvec4(leftFirst, 0.0f, leftIndex, 0.0f);
        nodes[leftIndex].triIndex_triCount_childIndex = uvec4(leftFirst, leftCount, -1.0f, 0.0f);
        nodes[rightIndex].triIndex_triCount_childIndex = uvec4(rightFirst, rightCount, -1.0f, 0.0f);

        updateNodeBounds(leftIndex);
        updateNodeBounds(rightIndex);

        split(leftIndex, depth + 1);
        split(rightIndex, depth + 1);
    }
}
