#include "VulkanSyncObjects.hpp"
#include "VulkanLogicalDevice.hpp"
#include "Debug/VulkanValidation.hpp"

namespace ve
{
    VulkanSyncObjects::VulkanSyncObjects(const VulkanLogicalDevice &logicalDevice, const SyncObjectsDesc &desc)
        : m_Device(logicalDevice.GetVkHandle())
    {
        VkSemaphoreCreateInfo semaphoreInfo{
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
        };

        VkFenceCreateInfo fenceInfo{
            .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
            .flags = desc.fenceSignaled ? VK_FENCE_CREATE_SIGNALED_BIT : 0u,
        };

        VkResult result = vkCreateSemaphore(m_Device, &semaphoreInfo, nullptr, &m_ImageAvailableSemaphore);
        CHECK_VK_RESULT(result);

        result = vkCreateSemaphore(m_Device, &semaphoreInfo, nullptr, &m_RenderFinishedSemaphore);
        CHECK_VK_RESULT(result);

        result = vkCreateFence(m_Device, &fenceInfo, nullptr, &m_InFlightFence);
        CHECK_VK_RESULT(result);
    }

    VulkanSyncObjects::~VulkanSyncObjects()
    {
        if (m_ImageAvailableSemaphore != VK_NULL_HANDLE)
            vkDestroySemaphore(m_Device, m_ImageAvailableSemaphore, nullptr);

        if (m_RenderFinishedSemaphore != VK_NULL_HANDLE)
            vkDestroySemaphore(m_Device, m_RenderFinishedSemaphore, nullptr);

        if (m_InFlightFence != VK_NULL_HANDLE)
            vkDestroyFence(m_Device, m_InFlightFence, nullptr);
    }

    void VulkanSyncObjects::WaitForFence() const
    {
        VkResult result = vkWaitForFences(m_Device, 1, &m_InFlightFence, VK_TRUE, UINT64_MAX);
        CHECK_VK_RESULT(result);
    }

    void VulkanSyncObjects::ResetFence() const
    {
        VkResult result = vkResetFences(m_Device, 1, &m_InFlightFence);
        CHECK_VK_RESULT(result);
    }

} // namespace ve
