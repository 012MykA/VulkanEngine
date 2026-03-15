#include "VulkanFramebuffer.hpp"
#include "Debug/VulkanValidation.hpp"

namespace ve
{
    VulkanFramebuffer::VulkanFramebuffer(
        VkDevice device, VkRenderPass renderPass, VkExtent2D extent,
        const std::vector<VkImageView> &attachments) : m_Device(device)
    {
        VkFramebufferCreateInfo createInfo{
            .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
            .renderPass = renderPass,
            .attachmentCount = static_cast<uint32_t>(attachments.size()),
            .pAttachments = attachments.data(),
            .width = extent.width,
            .height = extent.height,
            .layers = 1,
        };

        VkResult result = vkCreateFramebuffer(m_Device, &createInfo, nullptr, &m_Framebuffer);
        CHECK_VK_RESULT(result);
    }

    VulkanFramebuffer::~VulkanFramebuffer()
    {
        vkDestroyFramebuffer(m_Device, m_Framebuffer, nullptr);
    }

} // namespace ve
