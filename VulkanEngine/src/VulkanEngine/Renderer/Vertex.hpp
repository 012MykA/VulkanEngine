#pragma once

#include <glm/glm.hpp>

namespace ve
{
    struct Vertex
    {
        glm::vec3 position;
        glm::vec3 normal;
        glm::vec4 tangent;
        glm::vec2 uv;
    };

} // namespace ve
