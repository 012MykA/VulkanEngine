#pragma once

#include <vulkan/vulkan.h>

namespace ve
{
    class VulkanLogicalDevice;

    struct SyncObjectsDesc
    {
        bool fenceSignaled = true;
    };

    class VulkanSyncObjects
    {
    public:
        VulkanSyncObjects(const VulkanLogicalDevice &logicalDevice,
                          const SyncObjectsDesc &desc = {});
        ~VulkanSyncObjects();

        VulkanSyncObjects(const VulkanSyncObjects &) = delete;
        VulkanSyncObjects &operator=(const VulkanSyncObjects &) = delete;

        VkSemaphore GetImageAvailableSemaphore() const { return m_ImageAvailableSemaphore; }
        VkSemaphore GetRenderFinishedSemaphore() const { return m_RenderFinishedSemaphore; }
        VkFence GetInFlightFence() const { return m_InFlightFence; }

        void WaitForFence() const;
        void ResetFence() const;

    private:
        VkSemaphore m_ImageAvailableSemaphore = VK_NULL_HANDLE;
        VkSemaphore m_RenderFinishedSemaphore = VK_NULL_HANDLE;
        VkFence m_InFlightFence = VK_NULL_HANDLE;

        VkDevice m_Device = VK_NULL_HANDLE;
    };

} // namespace ve
