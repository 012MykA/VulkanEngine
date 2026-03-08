#pragma once

#include <vulkan/vulkan.h>

namespace ve
{
    class VulkanRenderPass
    {
    public:
        VulkanRenderPass(VkDevice device, VkFormat swapchainImageFormat);
        ~VulkanRenderPass();

        VkRenderPass GetRenderPass() const { return m_RenderPass; }

    private:
        VkDevice m_Device;
        VkRenderPass m_RenderPass;
    };

} // namespace ve
