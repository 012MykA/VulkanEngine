#pragma once

#include <vulkan/vulkan.h>

#include <functional>

namespace ve
{
    class VulkanLogicalDevice;
    class VulkanCommandPool;

    class VulkanImmediateSubmit
    {
        using RecordFn = std::function<void(VkCommandBuffer)>;

    public:
        VulkanImmediateSubmit(const VulkanLogicalDevice &logicalDevice,
                              const VulkanCommandPool &commandPool,
                              VkQueue queue);

        ~VulkanImmediateSubmit();

        VulkanImmediateSubmit(const VulkanImmediateSubmit &) = delete;
        VulkanImmediateSubmit &operator=(const VulkanImmediateSubmit &) = delete;

        void Submit(const RecordFn &fn) const;

    private:
        VkDevice m_Device = VK_NULL_HANDLE;
        VkCommandPool m_CommandPool = VK_NULL_HANDLE;
        VkQueue m_Queue = VK_NULL_HANDLE;
        VkCommandBuffer m_CmdBuffer = VK_NULL_HANDLE;
        VkFence m_Fence = VK_NULL_HANDLE;
    };

} // namespace ve
