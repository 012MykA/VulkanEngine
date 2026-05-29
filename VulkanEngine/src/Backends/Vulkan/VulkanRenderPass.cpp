#include "VulkanRenderPass.hpp"
#include "VulkanLogicalDevice.hpp"
#include "Debug/VulkanValidation.hpp"

#include <array>
#include <vector>

namespace ve
{
    VulkanRenderPass::VulkanRenderPass(const VulkanLogicalDevice &logicalDevice,
                                       VkFormat swapchainFormat,
                                       VkFormat depthFormat,
                                       VkSampleCountFlagBits samples)
        : m_Device(logicalDevice.GetVkHandle())
    {
        const bool hasDepth = (depthFormat != VK_FORMAT_UNDEFINED);
        const bool hasMsaa = (samples != VK_SAMPLE_COUNT_1_BIT);

        // Attachments

        // attachment 0
        VkAttachmentDescription colorAttachment{
            .format = swapchainFormat,
            .samples = samples,
            .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp = hasMsaa
                           ? VK_ATTACHMENT_STORE_OP_DONT_CARE
                           : VK_ATTACHMENT_STORE_OP_STORE,
            .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .finalLayout = hasMsaa
                               ? VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
                               : VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
        };
        VkAttachmentReference colorRef{
            .attachment = 0,
            .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        };

        // attachment 1
        VkAttachmentDescription depthAttachment{
            .format = depthFormat,
            .samples = samples,
            .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
        };
        VkAttachmentReference depthRef{
            .attachment = 1,
            .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
        };

        // attachment 2
        VkAttachmentDescription resolveAttachment{
            .format = swapchainFormat,
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
            .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
        };
        VkAttachmentReference resolveRef{
            .attachment = 2,
            .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        };

        // Subpass
        VkSubpassDescription subpass{
            .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
            .colorAttachmentCount = 1,
            .pColorAttachments = &colorRef,
            .pResolveAttachments = hasMsaa ? &resolveRef : nullptr,
            .pDepthStencilAttachment = hasDepth ? &depthRef : nullptr,
        };

        // Dependencies
        std::array<VkSubpassDependency, 2> dependencies{};

        // Color
        dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
        dependencies[0].dstSubpass = 0;
        dependencies[0].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependencies[0].srcAccessMask = 0;
        dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

        // Depth
        dependencies[1].srcSubpass = VK_SUBPASS_EXTERNAL;
        dependencies[1].dstSubpass = 0;
        dependencies[1].srcStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT |
                                       VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
        dependencies[1].dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT |
                                       VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
        dependencies[1].srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        dependencies[1].dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
                                        VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

        // RenderPass
        std::vector<VkAttachmentDescription> attachments = {colorAttachment};
        if (hasDepth)
            attachments.push_back(depthAttachment);
        if (hasMsaa)
            attachments.push_back(resolveAttachment);

        VkRenderPassCreateInfo createInfo{
            .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
            .attachmentCount = static_cast<uint32_t>(attachments.size()),
            .pAttachments = attachments.data(),
            .subpassCount = 1,
            .pSubpasses = &subpass,
            .dependencyCount = static_cast<uint32_t>(dependencies.size()),
            .pDependencies = dependencies.data(),
        };

        VkResult result = vkCreateRenderPass(m_Device, &createInfo, nullptr, &m_RenderPass);
        CHECK_VK_RESULT(result);
    }

    VulkanRenderPass::~VulkanRenderPass()
    {
        if (m_RenderPass != VK_NULL_HANDLE)
            vkDestroyRenderPass(m_Device, m_RenderPass, nullptr);
    }

} // namespace ve
