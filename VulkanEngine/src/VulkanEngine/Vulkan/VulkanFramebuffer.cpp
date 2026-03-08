#include "VulkanFramebuffer.hpp"
#include "Debug/Validation.hpp"

namespace ve
{
    VulkanFramebuffer::VulkanFramebuffer(
        VkDevice device, VkRenderPass renderPass, VkExtent2D extent,
        const std::vector<VkImageView> &attachments) : m_Device(device)
    {
        VkFramebufferCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        createInfo.renderPass = renderPass;
        createInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        createInfo.pAttachments = attachments.data();
        createInfo.width = extent.width;
        createInfo.height = extent.height;
        createInfo.layers = 1;

        VkResult result = vkCreateFramebuffer(m_Device, &createInfo, nullptr, &m_Framebuffer);
        CHECK_VK_RESULT(result);
    }

    VulkanFramebuffer::~VulkanFramebuffer()
    {
        vkDestroyFramebuffer(m_Device, m_Framebuffer, nullptr);
    }

} // namespace ve
