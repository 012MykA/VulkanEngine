#include "VulkanLogUtils.hpp"

namespace ve
{
    std::string VulkanDeviceTypeToString(VkPhysicalDeviceType deviceType)
    {
        switch (deviceType)
        {
        case VK_PHYSICAL_DEVICE_TYPE_OTHER:
            return "OTHER";
        case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
            return "INTEGRATED_GPU";
        case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
            return "DISCRETE_GPU";
        case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
            return "VIRTUAL_GPU";
        case VK_PHYSICAL_DEVICE_TYPE_CPU:
            return "CPU";
        }

        return "UNKNOWN";
    }

} // namespace ve
