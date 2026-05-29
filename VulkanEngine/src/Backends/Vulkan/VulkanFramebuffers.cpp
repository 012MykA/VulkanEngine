#include "VulkanFramebuffers.hpp"
#include "VulkanLogicalDevice.hpp"
#include "VulkanSwapchain.hpp"
#include "VulkanRenderPass.hpp"
#include "VulkanDepthBuffer.hpp"
#include "Debug/VulkanValidation.hpp"
#include "VulkanEngine/Core/Log.hpp"

#include <vector>

namespace ve
{
    VulkanFramebuffers::VulkanFramebuffers(
        const VulkanLogicalDevice &logicalDevice,
        const VulkanSwapchain &swapchain,
        const VulkanRenderPass &renderPass,
        const VulkanDepthBuffer &depthBuffer,
        VkImageView msaaColorView)
        : m_Device(logicalDevice.GetVkHandle())
    {
        Create(swapchain, renderPass, depthBuffer, msaaColorView);
    }

    VulkanFramebuffers::~VulkanFramebuffers()
    {
        Destroy();
    }

    void VulkanFramebuffers::Recreate(
        const VulkanSwapchain &swapchain,
        const VulkanRenderPass &renderPass,
        const VulkanDepthBuffer &depthBuffer,
        VkImageView msaaColorView)
    {
        Destroy();
        Create(swapchain, renderPass, depthBuffer, msaaColorView);
    }

    void VulkanFramebuffers::Create(const VulkanSwapchain &swapchain,
                                    const VulkanRenderPass &renderPass,
                                    const VulkanDepthBuffer &depthBuffer,
                                    VkImageView msaaColorView)
    {
        const auto &imageViews = swapchain.GetImageViews();
        const VkExtent2D &extent = swapchain.GetExtent();
        m_Framebuffers.resize(imageViews.size());

        for (size_t i = 0; i < imageViews.size(); ++i)
        {
            std::vector<VkImageView> attachments;

            if (msaaColorView != VK_NULL_HANDLE)
            {
                // MSAA: [msaaColor, depth, resolve(swapchain)]
                attachments = {msaaColorView, depthBuffer.GetView(), imageViews[i]};
            }
            else
            {
                // No MSAA: [swapchainColor, depth]
                attachments = {imageViews[i], depthBuffer.GetView()};
            }

            VkFramebufferCreateInfo framebufferInfo{
                .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
                .renderPass = renderPass.GetVkHandle(),
                .attachmentCount = static_cast<uint32_t>(attachments.size()),
                .pAttachments = attachments.data(),
                .width = extent.width,
                .height = extent.height,
                .layers = 1,
            };

            VkResult result = vkCreateFramebuffer(m_Device, &framebufferInfo, nullptr, &m_Framebuffers[i]);
            CHECK_VK_RESULT(result);
        }
    }

    void VulkanFramebuffers::Destroy()
    {
        for (auto framebuffer : m_Framebuffers)
            vkDestroyFramebuffer(m_Device, framebuffer, nullptr);

        m_Framebuffers.clear();
    }

} // namespace ve
