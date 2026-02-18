#include "FrameBuffer.hpp"
#include "Device.hpp"
#include "RenderPass.hpp"
#include "Validation.hpp"

#include <array>

namespace VE
{
    Framebuffer::Framebuffer(const Device &device,
                             const RenderPass &renderPass,
                             VkImageView imageView,
                             VkExtent2D extent)
        : m_Device(device)
    {
        std::array<VkImageView, 1> attachments = {imageView};

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = renderPass.Handle();
        framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        framebufferInfo.pAttachments = attachments.data();
        framebufferInfo.width = extent.width;
        framebufferInfo.height = extent.height;
        framebufferInfo.layers = 1;

        CheckVk(vkCreateFramebuffer(m_Device.Handle(), &framebufferInfo, nullptr, &m_Framebuffer), "create framebuffer!");
    }

    Framebuffer::Framebuffer(Framebuffer &&other) noexcept : m_Device(other.m_Device), m_Framebuffer(other.m_Framebuffer)
    {
        other.m_Framebuffer = VK_NULL_HANDLE;
    }

    Framebuffer::~Framebuffer()
    {
        vkDestroyFramebuffer(m_Device.Handle(), m_Framebuffer, nullptr);
    }
}
