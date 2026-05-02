#pragma once

#include <glm/glm.hpp>

#include <limits>
#include <array>

namespace ve
{
    struct AABB
    {
        glm::vec3 min{std::numeric_limits<float>::max()};
        glm::vec3 max{std::numeric_limits<float>::lowest()};

        glm::vec3 Center() const { return (min + max) * 0.5f; }
        glm::vec3 Extents() const { return (max - min) * 0.5f; }

        void Expand(const glm::vec3 &p)
        {
            min = glm::min(min, p);
            max = glm::max(max, p);
        }

        void Expand(const AABB &other)
        {
            min = glm::min(min, other.min);
            max = glm::max(max, other.max);
        }

        static AABB GetWorldAABB(const AABB &localAABB, const glm::mat4 &worldTransform)
        {
            glm::vec3 min = localAABB.min;
            glm::vec3 max = localAABB.max;

            std::array<glm::vec3, 8> corners = {
                glm::vec3(min.x, min.y, min.z),
                glm::vec3(min.x, min.y, max.z),
                glm::vec3(min.x, max.y, min.z),
                glm::vec3(min.x, max.y, max.z),
                glm::vec3(max.x, min.y, min.z),
                glm::vec3(max.x, min.y, max.z),
                glm::vec3(max.x, max.y, min.z),
                glm::vec3(max.x, max.y, max.z),
            };

            AABB worldAABB;
            for (const auto &corner : corners)
            {
                worldAABB.Expand(glm::vec3(worldTransform * glm::vec4(corner, 1.0f)));
            }
            return worldAABB;
        }
    };

} // namespace ve
