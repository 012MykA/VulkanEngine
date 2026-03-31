#include "Renderer.hpp"
#include "VulkanEngine/Core/Window.hpp"
#include "VulkanEngine/Core/Log.hpp"
#include "VulkanEngine/Core/Timer.hpp"
#include "VulkanEngine/Vulkan/Debug/VulkanValidation.hpp"

namespace ve
{
    Renderer::Renderer(const Window &window)
    {
        Init(window);
    }

    Renderer::~Renderer()
    {
        m_LogicalDevice->WaitIdle();
    }

    void Renderer::BeginFrame()
    {
        auto &frame = m_FrameManager->GetCurrentFrame();

        frame.syncObjects->WaitForFence();

        VkResult result = vkAcquireNextImageKHR(
            m_LogicalDevice->GetVkHandle(),
            m_Swapchain->GetVkHandle(),
            UINT64_MAX,
            frame.syncObjects->GetImageAvailableSemaphore(),
            VK_NULL_HANDLE,
            &m_CurrentImageIndex);

        if (result == VK_ERROR_OUT_OF_DATE_KHR)
            RecreateSwapchain();

        if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
        {
            CHECK_VK_RESULT(result);
        }

        frame.syncObjects->ResetFence();

        VkCommandBuffer cmd = frame.commandBuffer;
        vkResetCommandBuffer(cmd, 0);

        VkCommandBufferBeginInfo beginInfo{
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
        };

        result = vkBeginCommandBuffer(cmd, &beginInfo);
        CHECK_VK_RESULT(result);

        // RenderPass
        VkClearValue clearColor = {
            .color = {{0.1f, 0.1f, 0.1f, 1.0f}},
        };

        VkRenderPassBeginInfo rpBegin{
            .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
            .renderPass = m_RenderPass->GetVkHandle(),
            .framebuffer = m_Framebuffers->GetFramebuffer(m_CurrentImageIndex),
            .renderArea{
                .offset = {0, 0},
                .extent = m_Swapchain->GetExtent(),
            },
            .clearValueCount = 1,
            .pClearValues = &clearColor,
        };

        vkCmdBeginRenderPass(cmd, &rpBegin, VK_SUBPASS_CONTENTS_INLINE);

        // Dynamic viewport and scissor
        VkExtent2D extent = m_Swapchain->GetExtent();

        VkViewport viewport{
            .x = 0.0f,
            .y = 0.0f,
            .width = static_cast<float>(extent.width),
            .height = static_cast<float>(extent.height),
            .minDepth = 0.0f,
            .maxDepth = 1.0f,
        };
        vkCmdSetViewport(cmd, 0, 1, &viewport);

        VkRect2D scissor{
            .offset = {0, 0},
            .extent = extent,
        };
        vkCmdSetScissor(cmd, 0, 1, &scissor);
    }

    void Renderer::EndFrame()
    {
        auto &frame = m_FrameManager->GetCurrentFrame();
        VkCommandBuffer cmd = frame.commandBuffer;

        // End render pass and command buffer
        vkCmdEndRenderPass(cmd);
        VkResult result = vkEndCommandBuffer(cmd);
        CHECK_VK_RESULT(result);

        // Submit
        VkSemaphore waitSem = frame.syncObjects->GetImageAvailableSemaphore();
        VkSemaphore signalSem = frame.syncObjects->GetRenderFinishedSemaphore();
        VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

        VkSubmitInfo submitInfo{
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = &waitSem,
            .pWaitDstStageMask = &waitStage,
            .commandBufferCount = 1,
            .pCommandBuffers = &cmd,
            .signalSemaphoreCount = 1,
            .pSignalSemaphores = &signalSem,
        };

        result = vkQueueSubmit(m_LogicalDevice->GetGraphicsQueue(), 1, &submitInfo, frame.syncObjects->GetInFlightFence());
        CHECK_VK_RESULT(result);

        // Present
        VkSwapchainKHR swapchains[] = {m_Swapchain->GetVkHandle()};

        VkPresentInfoKHR presentInfo{
            .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = &signalSem,
            .swapchainCount = 1,
            .pSwapchains = swapchains,
            .pImageIndices = &m_CurrentImageIndex,
        };

        result = vkQueuePresentKHR(m_LogicalDevice->GetPresentQueue(), &presentInfo);

        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_NeedsResize)
        {
            m_NeedsResize = false;
            RecreateSwapchain();
        }
        else
        {
            CHECK_VK_RESULT(result);
        }

        m_FrameManager->AdvanceFrame();
    }

    void Renderer::HandleResize(uint32_t width, uint32_t height)
    {
        if (width == 0 || height == 0)
            return;

        m_NeedsResize = true;
        m_ResizeWidth = width;
        m_ResizeHeight = height;
    }

    void Renderer::Init(const Window &window)
    {
        // Instance
        m_Instance = std::make_unique<VulkanInstance>(InstanceDesc{
            .requiredExtensions = window.GetRequiredVulkanExtensions(),
#ifdef VE_DEBUG
            .enableValidation = true,
            .debugMessenger{
                .enableDebugMessenger = true,
                .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                   VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
            },
#endif
        });

        m_Surface = std::make_unique<VulkanSurface>(*m_Instance, window);
        m_PhysicalDevice = std::make_unique<VulkanPhysicalDevice>(*m_Instance, *m_Surface);
        m_LogicalDevice = std::make_unique<VulkanLogicalDevice>(*m_PhysicalDevice, LogicalDeviceDesc{});
        m_Allocator = std::make_unique<VulkanAllocator>(*m_Instance, *m_PhysicalDevice, *m_LogicalDevice);

        m_Swapchain = std::make_unique<VulkanSwapchain>(
            *m_PhysicalDevice,
            *m_LogicalDevice,
            *m_Surface,
            SwapchainDesc{
                .width = window.GetWidth(),
                .height = window.GetHeight(),
            });

        m_RenderPass = std::make_unique<VulkanRenderPass>(*m_LogicalDevice, m_Swapchain->GetFormat());
        m_Framebuffers = std::make_unique<VulkanFramebuffers>(*m_LogicalDevice, *m_Swapchain, *m_RenderPass);
        m_GraphicsCommandPool = std::make_unique<VulkanCommandPool>(
            *m_LogicalDevice, *m_PhysicalDevice,
            CommandPoolDesc{.type = CommandPoolType::Graphics, .resetBuffer = true});

        m_FrameManager = std::make_unique<VulkanFrameManager>(*m_LogicalDevice, *m_GraphicsCommandPool);
    }

    void Renderer::RecreateSwapchain()
    {
        m_LogicalDevice->WaitIdle();

        Timer timer;

        m_Swapchain->Recreate(m_ResizeWidth, m_ResizeHeight);
        m_Framebuffers->Recreate(*m_Swapchain, *m_RenderPass);

        VE_CORE_TRACE("Swapchain recreated: {}x{} ({} ms)", m_ResizeWidth, m_ResizeHeight, timer.ElapsedMilliseconds());
    }

} // namespace ve
