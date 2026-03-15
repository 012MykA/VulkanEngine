#pragma once

#include <vulkan/vulkan.h>
#include <glm/glm.hpp>

#include <array>

namespace ve
{
    struct Vertex
    {
        glm::vec3 Position;
        glm::vec3 Color;
        glm::vec2 TexCoord;

        static VkVertexInputBindingDescription GetBindingDescription();

        static std::array<VkVertexInputAttributeDescription, 3> GetAttributeDescriptions();
    };

} // namespace ve
