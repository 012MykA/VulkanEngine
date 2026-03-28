#include "VulkanImmediateSubmit.hpp"
#include "VulkanLogicalDevice.hpp"
#include "VulkanCommandPool.hpp"
#include "Debug/VulkanValidation.hpp"

namespace ve
{
    VulkanImmediateSubmit::VulkanImmediateSubmit(const VulkanLogicalDevice &logicalDevice,
                                                 const VulkanCommandPool &commandPool,
                                                 VkQueue queue)
        : m_Device(logicalDevice.GetVkHandle()), m_CommandPool(commandPool.GetVkHandle()), m_Queue(queue)
    {
        VkCommandBufferAllocateInfo allocInfo{
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .commandPool = m_CommandPool,
            .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .commandBufferCount = 1,
        };
        VkResult result = vkAllocateCommandBuffers(m_Device, &allocInfo, &m_CmdBuffer);
        CHECK_VK_RESULT(result);

        VkFenceCreateInfo fenceInfo{
            .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        };
        result = vkCreateFence(m_Device, &fenceInfo, nullptr, &m_Fence);
        CHECK_VK_RESULT(result);
    }

    VulkanImmediateSubmit::~VulkanImmediateSubmit()
    {
        if (m_Fence != VK_NULL_HANDLE)
            vkDestroyFence(m_Device, m_Fence, nullptr);

        if (m_CmdBuffer != VK_NULL_HANDLE)
            vkFreeCommandBuffers(m_Device, m_CommandPool, 1, &m_CmdBuffer);
    }

    void VulkanImmediateSubmit::Submit(const RecordFn &fn) const
    {
        // Reset
        vkResetFences(m_Device, 1, &m_Fence);
        vkResetCommandBuffer(m_CmdBuffer, 0);

        // Record
        VkCommandBufferBeginInfo beginInfo{
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
        };
        VkResult result = vkBeginCommandBuffer(m_CmdBuffer, &beginInfo);
        CHECK_VK_RESULT(result);

        fn(m_CmdBuffer);

        result = vkEndCommandBuffer(m_CmdBuffer);
        CHECK_VK_RESULT(result);

        // Submit
        VkCommandBufferSubmitInfo cmdSubmit{
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
            .commandBuffer = m_CmdBuffer,
        };

        VkSubmitInfo2 submitInfo{
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
            .commandBufferInfoCount = 1,
            .pCommandBufferInfos = &cmdSubmit,
        };

        result = vkQueueSubmit2(m_Queue, 1, &submitInfo, m_Fence);
        CHECK_VK_RESULT(result);

        // Wait for fences
        result = vkWaitForFences(m_Device, 1, &m_Fence, VK_TRUE, UINT64_MAX);
        CHECK_VK_RESULT(result);
    }

} // namespace ve
