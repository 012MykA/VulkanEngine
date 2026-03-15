#include "VulkanRenderPass.hpp"
#include "VulkanEngine/Core/Log.hpp"
#include "Debug/VulkanValidation.hpp"

namespace ve
{
    VulkanRenderPass::VulkanRenderPass(VkDevice device, VkFormat swapchainImageFormat, const std::string &debugName)
        : m_Device(device), m_DebugName(debugName)
    {
        // Color Attachment
        VkAttachmentDescription colorAttachment{
            .format = swapchainImageFormat,
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
            .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
        };

        // Refs
        VkAttachmentReference colorAttachmentRef{
            .attachment = 0, // Index in pAttachments array
            .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        };

        // Subpass
        VkSubpassDescription subpass{
            .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
            .colorAttachmentCount = 1,
            .pColorAttachments = &colorAttachmentRef,
            .pDepthStencilAttachment = nullptr,
        };

        // Dependency
        VkSubpassDependency dependency{
            .srcSubpass = VK_SUBPASS_EXTERNAL,
            .dstSubpass = 0,
            .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            .srcAccessMask = 0,
            .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
        };

        // Render pass info
        VkRenderPassCreateInfo renderPassInfo{
            .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
            .attachmentCount = 1,
            .pAttachments = &colorAttachment,
            .subpassCount = 1,
            .pSubpasses = &subpass,
            .dependencyCount = 1,
            .pDependencies = &dependency,
        };

        VkResult result = vkCreateRenderPass(m_Device, &renderPassInfo, nullptr, &m_RenderPass);
        CHECK_VK_RESULT(result);

        VE_CORE_TRACE("VkRenderPass ({0}) created", m_DebugName);
    }

    VulkanRenderPass::~VulkanRenderPass()
    {
        vkDestroyRenderPass(m_Device, m_RenderPass, nullptr);
        VE_CORE_TRACE("VkRenderPass ({0}) destroyed", m_DebugName);
    }

} // namespace ve
