#pragma once

#include <vulkan/vulkan.hpp>

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
        bool RequiresPresentQueue = true;
        std::vector<const char *> Extensions;
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

    class VulkanPhysicalDevices
    {
    public:
        VulkanPhysicalDevices() = default;
        ~VulkanPhysicalDevices() = default;

        void Init(VkInstance instance);

        const PhysicalDevice &Selected() const { return m_PhysicalDevices[m_SelectedDeviceIndex]; }
        void SelectDevice(VkSurfaceKHR surface, const PhysicalDeviceRequirements &requirements);

    private:
        // Helpers used during initialization / device selection
        void PickPhysicalDevice(VkSurfaceKHR surface, const PhysicalDeviceRequirements &requirements);
        bool IsDeviceSuitable(const PhysicalDevice &device, VkSurfaceKHR surface, const PhysicalDeviceRequirements &requirements) const;
        PhysicalDeviceQueueFamilyIndices FindQueueIndices(VkPhysicalDevice device, VkSurfaceKHR surface) const;
        bool CheckDeviceExtensionSupport(VkPhysicalDevice device, const std::vector<const char *> &requiredExtensions) const;

    private:
        std::vector<PhysicalDevice> m_PhysicalDevices;
        uint32_t m_SelectedDeviceIndex = 0;
    };

} // namespace ve
