#pragma once

#ifdef DeviceCapabilities
#undef DeviceCapabilities
#endif

#include "Debug/VulkanValidation.hpp"

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

    struct DeviceCapabilities
    {
        bool anisotropySupported = false;
        float maxAnisotropy = 1.0f;

        VkSampleCountFlags msaaSampleCounts = VK_SAMPLE_COUNT_1_BIT;

        bool geometryShaderSupported = false;
        bool tessellationShaderSupported = false;

        // Helpers
        VkSampleCountFlagBits ClampMSAASamples(VkSampleCountFlagBits requested) const
        {
            if (msaaSampleCounts & requested)
                return requested;

            VkSampleCountFlagBits result = VK_SAMPLE_COUNT_1_BIT;
            uint32_t s = static_cast<uint32_t>(requested);

            while (s > VK_SAMPLE_COUNT_1_BIT)
            {
                s >>= 1;

                if (msaaSampleCounts & s)
                {
                    result = static_cast<VkSampleCountFlagBits>(s);
                    break;
                }
            }

            VE_CORE_WARN("Requested MSAA samples {} not supported, falling back to {}",
                         string_VkSampleCountFlagBits(requested),
                         string_VkSampleCountFlagBits(static_cast<VkSampleCountFlagBits>(s)));
            return result;
        }

        float ClampAnisotropy(float requested) const
        {
            if (!anisotropySupported)
            {
                if (requested > 1.0f)
                    VE_CORE_WARN("Anisotropic filtering not supported. Disabling (fallback to 1.0)");

                return 1.0f;
            }

            if (requested > maxAnisotropy)
            {
                VE_CORE_WARN("Anisotropy: Requested x{:.1f} exceeds device limit. Clamping to x{:.1f}.",
                             requested, maxAnisotropy);
                return maxAnisotropy;
            }

            return requested;
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
        const DeviceCapabilities &GetCapabilities() const { return m_Capabilities; }

        SwapchainSupportDetails QuerySwapchainSupport() const;

    private:
        void PickPhysicalDevice(VkInstance instance, VkSurfaceKHR surface);
        void BuildCapabilities();

        int32_t RateDevice(VkPhysicalDevice device, VkSurfaceKHR surface) const; // 0 = not suitable
        bool CheckExtensionSupport(VkPhysicalDevice device) const;
        QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface) const;

    private:
        VkPhysicalDevice m_PhysicalDevice = VK_NULL_HANDLE;
        VkSurfaceKHR m_Surface = VK_NULL_HANDLE;
        QueueFamilyIndices m_QueueFamilies;
        VkPhysicalDeviceProperties m_Properties{};
        VkPhysicalDeviceFeatures m_Features{};
        DeviceCapabilities m_Capabilities{};
    };

} // namespace ve
