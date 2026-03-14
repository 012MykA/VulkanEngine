#pragma once

#include <vulkan/vulkan.h>

#include <cstdint>

namespace ve
{
    struct DescriptorBinding
    {
        uint32_t binding;
        uint32_t descriptorCount;
        VkDescriptorType type;
        VkShaderStageFlags stage;
    };

} // namespace ve
