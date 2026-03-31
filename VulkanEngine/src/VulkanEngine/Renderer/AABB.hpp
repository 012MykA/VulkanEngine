#pragma once

#include <glm/glm.hpp>

#include <limits>

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
    };

} // namespace ve
