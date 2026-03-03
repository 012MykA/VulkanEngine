#pragma once

#include <vulkan/vulkan.h>

#include <vector>

struct PhysicalDevice
{
    VkPhysicalDevice physicalDevice;
    VkPhysicalDeviceProperties deviceProps;
    std::vector<VkQueueFamilyProperties> queueFamilyProps;
    std::vector<VkBool32> queueSupportsPresent;
    std::vector<VkSurfaceFormatKHR> surfaceFormats;
    VkSurfaceCapabilitiesKHR surfaceCapabilities;
    VkPhysicalDeviceMemoryProperties memoryProps;
    std::vector<VkPresentModeKHR> presentModes;
};

namespace ve
{
    class VulkanPhysicalDevices
    {
    public:
        VulkanPhysicalDevices() = default;
        ~VulkanPhysicalDevices() = default;

        void Init(VkInstance instance, VkSurfaceKHR surface);

        uint32_t SelectDevice(VkQueueFlags requiredQueueType, bool supportsPresent);

        const PhysicalDevice &Selected() const;

    private:
        std::vector<PhysicalDevice> m_Devices;
        int m_SelectedDeviceIndex = -1;
    };

} // namespace ve
