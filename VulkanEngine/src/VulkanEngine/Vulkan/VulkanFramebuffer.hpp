#pragma once

#include <vulkan/vulkan.h>

#include <vector>

namespace ve
{
    class VulkanFramebuffer
    {
    public:
        VulkanFramebuffer(VkDevice device, VkRenderPass renderPass, VkExtent2D extent,
                          const std::vector<VkImageView> &attachments);
        ~VulkanFramebuffer();

        VkFramebuffer GetFramebuffer() const { return m_Framebuffer; }

    private:
        VkDevice m_Device;
        VkFramebuffer m_Framebuffer = VK_NULL_HANDLE;
    };

} // namespace ve
