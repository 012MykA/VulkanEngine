#pragma once

#include <vulkan/vulkan.hpp>

#include <vector>
#include <optional>

namespace VE
{
    class Instance;
    class Surface;

    struct QueueFamilyIndices
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

    class Device final
    {
    public:
        Device(const Instance &instance, const Surface &surface);
        ~Device();

        VkDevice Handle() const { return m_Device; }
        VkPhysicalDevice PhysicalDevice() const { return m_PhysicalDevice; }

        VkQueue GraphicsQueue() const { return m_GraphicsQueue; }
        VkQueue PresentQueue() const { return m_PresentQueue; }

        const QueueFamilyIndices &Queues() const { return m_QueueFamilies; }
        SwapchainSupportDetails QuerySwapchainSupport(VkPhysicalDevice device) const;

    private:
        void PickPhysicalDevice();
        void CreateLogicalDevice();

        bool IsDeviceSuitable(VkPhysicalDevice device);
        bool CheckDeviceExtensionSupport(VkPhysicalDevice device);
        QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device);

    private:
        const Instance &m_Instance;
        const Surface &m_Surface;

        VkPhysicalDevice m_PhysicalDevice = VK_NULL_HANDLE;
        VkDevice m_Device = VK_NULL_HANDLE;

        VkQueue m_GraphicsQueue = VK_NULL_HANDLE;
        VkQueue m_PresentQueue = VK_NULL_HANDLE;

        QueueFamilyIndices m_QueueFamilies;

        const std::vector<const char *> m_DeviceExtensions = {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME,
        };
    };

}
