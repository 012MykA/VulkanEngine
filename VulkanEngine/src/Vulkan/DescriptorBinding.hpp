#pragma once

#include <vulkan/vulkan.h>

#include <cstdint>

struct DescriptorBinding
{
    uint32_t Binding;
    uint32_t DescriptorCount;
    VkDescriptorType Type;
    VkShaderStageFlags Stage;
};
