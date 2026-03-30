#pragma once

#include <vulkan/vulkan.h>

namespace ve
{
    class VulkanLogicalDevice;

    class VulkanRenderPass
    {
    public:
        VulkanRenderPass(const VulkanLogicalDevice &logicalDevice, VkFormat swapchainFormat);
        ~VulkanRenderPass();

        VulkanRenderPass(const VulkanRenderPass &) = delete;
        VulkanRenderPass &operator=(const VulkanRenderPass &) = delete;

        VkRenderPass GetVkHandle() const { return m_RenderPass; }

    private:
        VkDevice m_Device = VK_NULL_HANDLE;
        VkRenderPass m_RenderPass = VK_NULL_HANDLE;
    };

} // namespace ve
