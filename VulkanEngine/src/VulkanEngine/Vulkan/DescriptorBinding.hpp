#pragma once

#include <vulkan/vulkan.h>

#include <cstdint>

namespace ve
{
    struct DescriptorBinding
    {
        uint32_t Binding;
        uint32_t DescriptorCount;
        VkDescriptorType Type;
        VkShaderStageFlags Stage;
    };

} // namespace ve
