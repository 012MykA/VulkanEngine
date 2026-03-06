#pragma once

#include <vulkan/vulkan.h>

#include <string>

namespace ve
{
    std::string VulkanDeviceTypeToString(VkPhysicalDeviceType deviceType);
    
} // namespace ve
