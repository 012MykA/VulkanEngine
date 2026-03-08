#pragma once

#include <vulkan/vulkan.h>

#include <vector>
#include <cstdint>
#include <optional>

namespace ve
{
    struct PhysicalDeviceRequirements
    {
        bool RequiresGraphicsQueue = false;
        bool RequiresPresentQueue = false;
        bool SwapchainAdequate = false;
        std::vector<const char *> Extensions;
        VkPhysicalDeviceFeatures Features{};
        std::optional<VkPhysicalDeviceType> PreferredDeviceType;
    };

    struct PhysicalDeviceQueueFamilyIndices
    {
        std::optional<uint32_t> GraphicsFamily;
        std::optional<uint32_t> PresentFamily;

        bool IsComplete() const { return GraphicsFamily.has_value() && PresentFamily.has_value(); }
    };

    struct SwapchainSupportDetails
    {
        VkSurfaceCapabilitiesKHR Capabilities;
        std::vector<VkSurfaceFormatKHR> Formats;
        std::vector<VkPresentModeKHR> PresentModes;
    };

    class VulkanPhysicalDevice
    {
    public:
        static VulkanPhysicalDevice Select(VkInstance instance, VkSurfaceKHR surface, const PhysicalDeviceRequirements &req);

        ~VulkanPhysicalDevice() = default;
        VulkanPhysicalDevice(VkPhysicalDevice device, VkSurfaceKHR surface);

    public:
        // Getters
        VkPhysicalDevice GetVulkanHandle() const { return m_Device; }
        const VkPhysicalDeviceProperties &GetProperties() const { return m_Properties; }
        const VkPhysicalDeviceFeatures &GetFeatures() const { return m_Features; }
        const VkPhysicalDeviceMemoryProperties &GetMemoryProperties() const { return m_MemoryProperties; }
        const PhysicalDeviceQueueFamilyIndices &GetQueueIndices() const { return m_QueueIndices; }

        SwapchainSupportDetails GetSwapchainSupport(VkSurfaceKHR surface) const;

    private:
        static bool IsDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR surface, const PhysicalDeviceRequirements &req);
        static bool CheckExtensionSupport(VkPhysicalDevice device, const std::vector<const char *> &requiredExtensions);
        static bool CheckFeatureSupport(const VkPhysicalDeviceFeatures &supported, const VkPhysicalDeviceFeatures &required);
        static PhysicalDeviceQueueFamilyIndices FindQueueIndices(VkPhysicalDevice device, VkSurfaceKHR surface);
        static SwapchainSupportDetails QuerySwapchainSupport(VkPhysicalDevice device, VkSurfaceKHR surface);

    private:
        VkPhysicalDevice m_Device = VK_NULL_HANDLE;
        VkPhysicalDeviceProperties m_Properties;
        VkPhysicalDeviceFeatures m_Features;
        VkPhysicalDeviceMemoryProperties m_MemoryProperties;
        PhysicalDeviceQueueFamilyIndices m_QueueIndices;
    };

} // namespace ve
