#pragma once

#include <vulkan/vulkan.h>

#include <optional>
#include <cstdint>
#include <vector>

namespace ve
{
    class VulkanInstance;
    class VulkanSurface;

    struct QueueFamilyIndices
    {
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> presentFamily;
        std::optional<uint32_t> computeFamily;
        std::optional<uint32_t> transferFamily; // Dedicated transfer

        bool IsComplete() const
        {
            return graphicsFamily.has_value() &&
                   presentFamily.has_value() &&
                   computeFamily.has_value();
        }

        uint32_t GetTransferFamily() const
        {
            return transferFamily.value_or(graphicsFamily.value());
        }
    };

    struct SwapchainSupportDetails
    {
        VkSurfaceCapabilitiesKHR capabilities{};
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;

        bool IsAdequate() const
        {
            return !formats.empty() && !presentModes.empty();
        }
    };

    class VulkanPhysicalDevice
    {
    public:
        static constexpr const char *k_RequiredExtensions[] = {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME,
        };

        VulkanPhysicalDevice(const VulkanInstance &instance, const VulkanSurface &surface);

        VulkanPhysicalDevice(const VulkanPhysicalDevice &) = delete;
        VulkanPhysicalDevice &operator=(const VulkanPhysicalDevice &) = delete;

    public:
        // Getters
        VkPhysicalDevice GetVkHandle() const { return m_PhysicalDevice; }
        const QueueFamilyIndices &GetQueueFamilies() const { return m_QueueFamilies; }
        const VkPhysicalDeviceProperties &GetProperties() const { return m_Properties; }
        const VkPhysicalDeviceFeatures &GetFeatures() const { return m_Features; }

        SwapchainSupportDetails QuerySwapchainSupport() const;

    private:
        void PickPhysicalDevice(VkInstance instance, VkSurfaceKHR surface);
        int32_t RateDevice(VkPhysicalDevice device, VkSurfaceKHR surface) const; // 0 = not suitable

        bool CheckExtensionSupport(VkPhysicalDevice device) const;
        QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface) const;

    private:
        VkPhysicalDevice m_PhysicalDevice = VK_NULL_HANDLE;
        VkSurfaceKHR m_Surface = VK_NULL_HANDLE;
        QueueFamilyIndices m_QueueFamilies;
        VkPhysicalDeviceProperties m_Properties{};
        VkPhysicalDeviceFeatures m_Features{};
    };

} // namespace ve
