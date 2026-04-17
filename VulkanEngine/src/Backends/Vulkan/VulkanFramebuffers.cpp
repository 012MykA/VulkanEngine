#include "VulkanFramebuffers.hpp"
#include "VulkanLogicalDevice.hpp"
#include "VulkanSwapchain.hpp"
#include "VulkanRenderPass.hpp"
#include "VulkanDepthBuffer.hpp"
#include "Debug/VulkanValidation.hpp"
#include "VulkanEngine/Core/Log.hpp"

namespace ve
{
    VulkanFramebuffers::VulkanFramebuffers(
        const VulkanLogicalDevice &logicalDevice,
        const VulkanSwapchain &swapchain,
        const VulkanRenderPass &renderPass,
        const VulkanDepthBuffer &depthBuffer)
        : m_Device(logicalDevice.GetVkHandle())
    {
        Create(swapchain, renderPass, depthBuffer);
    }

    VulkanFramebuffers::~VulkanFramebuffers()
    {
        Destroy();
    }

    void VulkanFramebuffers::Recreate(const VulkanSwapchain &swapchain,
                                      const VulkanRenderPass &renderPass,
                                      const VulkanDepthBuffer &depthBuffer)
    {
        Destroy();
        Create(swapchain, renderPass, depthBuffer);
    }

    void VulkanFramebuffers::Create(const VulkanSwapchain &swapchain,
                                    const VulkanRenderPass &renderPass,
                                    const VulkanDepthBuffer &depthBuffer)
    {
        const auto &imageViews = swapchain.GetImageViews();
        const VkExtent2D &extent = swapchain.GetExtent();

        m_Framebuffers.resize(imageViews.size());

        for (size_t i = 0; i < imageViews.size(); ++i)
        {
            VkImageView attachments[] = {
                imageViews[i],
                depthBuffer.GetView(),
            };

            VkFramebufferCreateInfo framebufferInfo{
                .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
                .renderPass = renderPass.GetVkHandle(),
                .attachmentCount = 2,
                .pAttachments = attachments,
                .width = extent.width,
                .height = extent.height,
                .layers = 1,
            };

            VkResult result = vkCreateFramebuffer(m_Device, &framebufferInfo, nullptr, &m_Framebuffers[i]);
            CHECK_VK_RESULT(result);
        }

        VE_CORE_TRACE("Framebuffers created ({})", m_Framebuffers.size());
    }

    void VulkanFramebuffers::Destroy()
    {
        for (auto framebuffer : m_Framebuffers)
            vkDestroyFramebuffer(m_Device, framebuffer, nullptr);

        m_Framebuffers.clear();
    }

} // namespace ve
