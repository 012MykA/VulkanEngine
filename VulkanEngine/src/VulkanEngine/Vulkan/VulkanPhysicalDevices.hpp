#pragma once

#include <vulkan/vulkan.h>

#include <vector>
#include <cstdint>
#include <optional>

namespace ve
{
    struct PhysicalDevice
    {
        VkPhysicalDevice Device;
        VkPhysicalDeviceProperties Properties;
        VkPhysicalDeviceFeatures Features;
        VkPhysicalDeviceMemoryProperties MemoryProperties;
    };

    struct PhysicalDeviceRequirements
    {
        bool RequiresGraphicsQueue = true;
        bool RequiresPresentQueue = false;
        bool SwapchainAdequate = true;
        VkPhysicalDeviceType PreferredDeviceType = VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
        std::vector<const char *> Extensions;
        VkPhysicalDeviceFeatures Features{};
    };

    struct PhysicalDeviceQueueFamilyIndices
    {
        std::optional<uint32_t> GraphicsFamily;
        std::optional<uint32_t> PresentFamily;

        bool IsComplete() const
        {
            return GraphicsFamily.has_value() && PresentFamily.has_value();
        }
    };

    struct SwapchainSupportDetails
    {
        VkSurfaceCapabilitiesKHR Capabilities;
        std::vector<VkSurfaceFormatKHR> Formats;
        std::vector<VkPresentModeKHR> PresentModes;
    };

    class VulkanPhysicalDevices
    {
    public:
        VulkanPhysicalDevices() = default;
        ~VulkanPhysicalDevices() = default;

        void Init(VkInstance instance);

        const PhysicalDevice &Selected() const { return m_PhysicalDevices[m_SelectedDeviceIndex]; }
        void SelectDevice(VkSurfaceKHR surface, const PhysicalDeviceRequirements &requirements);
        PhysicalDeviceQueueFamilyIndices GetQueueIndices(VkSurfaceKHR surface) const;

    private:
        bool IsDeviceSuitable(const PhysicalDevice &device, VkSurfaceKHR surface, const PhysicalDeviceRequirements &requirements) const;

        PhysicalDeviceQueueFamilyIndices FindQueueIndices(VkPhysicalDevice device, VkSurfaceKHR surface) const;
        bool CheckDeviceExtensionSupport(VkPhysicalDevice device, const std::vector<const char *> &requiredExtensions) const;
        bool CheckDeviceFeatureSupport(const VkPhysicalDeviceFeatures &supported, const VkPhysicalDeviceFeatures &required) const;
        SwapchainSupportDetails QuerySwapchainSupport(VkPhysicalDevice device, VkSurfaceKHR surface) const;

    private:
        std::vector<PhysicalDevice> m_PhysicalDevices;
        uint32_t m_SelectedDeviceIndex = 0;
    };

} // namespace ve
