#pragma once

#include "VulkanEngine/Renderer/Primitives/AABB.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_access.hpp>

#include <array>

namespace ve
{
    struct Frustum
    {
        glm::vec4 planes[6];

        static Frustum FromViewProjection(const glm::mat4 &vp)
        {
            Frustum f;
            // Left
            f.planes[0] = glm::row(vp, 3) + glm::row(vp, 0);
            // Right
            f.planes[1] = glm::row(vp, 3) - glm::row(vp, 0);
            // Bottom
            f.planes[2] = glm::row(vp, 3) + glm::row(vp, 1);
            // Top
            f.planes[3] = glm::row(vp, 3) - glm::row(vp, 1);
            // Near
            f.planes[4] = glm::row(vp, 3) + glm::row(vp, 2);
            // Far
            f.planes[5] = glm::row(vp, 3) - glm::row(vp, 2);

            for (auto &p : f.planes)
            {
                float len = glm::length(glm::vec3(p));
                p /= len;
            }
            return f;
        }

        bool TestAABB(const AABB &aabb) const
        {
            for (const auto &plane : planes)
            {
                glm::vec3 n(plane);

                glm::vec3 pv{
                    n.x >= 0.0f ? aabb.max.x : aabb.min.x,
                    n.y >= 0.0f ? aabb.max.y : aabb.min.y,
                    n.z >= 0.0f ? aabb.max.z : aabb.min.z,
                };
                if (glm::dot(n, pv) + plane.w < 0.0f)
                    return false;
            }
            return true;
        }
    };

} // namespace ve
