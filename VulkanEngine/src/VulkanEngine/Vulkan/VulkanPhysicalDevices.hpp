#pragma once

#include <vulkan/vulkan.hpp>

#include <vector>
#include <cstdint>

namespace ve
{
    struct PhysicalDevice
    {
        VkPhysicalDevice Device;
        VkPhysicalDeviceProperties Properties;
        VkPhysicalDeviceFeatures Features;
        VkPhysicalDeviceMemoryProperties MemoryProperties;
    };
    
    class VulkanPhysicalDevices
    {
    public:
        VulkanPhysicalDevices() = default;
        ~VulkanPhysicalDevices() = default;

        void Init(VkInstance instance, VkSurfaceKHR surface);

        const PhysicalDevice& Selected() const { return m_PhysicalDevices[m_SelectedDeviceIndex]; }

    private:
        std::vector<PhysicalDevice> m_PhysicalDevices;
        uint32_t m_SelectedDeviceIndex = 0;
    };

} // namespace ve
