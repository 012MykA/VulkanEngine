#pragma once

#include <vulkan/vulkan.h>

#include <string>

namespace ve
{
    class VulkanRenderPass
    {
    public:
        VulkanRenderPass(VkDevice device, VkFormat swapchainImageFormat, const std::string &debugName = "Unnamed");
        ~VulkanRenderPass();

        VkRenderPass GetRenderPass() const { return m_RenderPass; }

    private:
        VkDevice m_Device;
        VkRenderPass m_RenderPass;

        std::string m_DebugName;
    };

} // namespace ve
