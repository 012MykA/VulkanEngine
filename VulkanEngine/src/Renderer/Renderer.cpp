#include "Renderer.hpp"
#include "Window.hpp"
#include "Instance.hpp"
#include "Surface.hpp"
#include "Device.hpp"
#include "Swapchain.hpp"
#include "RenderPass.hpp"
#include "GraphicsPipeline.hpp"
#include "CommandPool.hpp"
#include "CommandBuffers.hpp"
#include "Validation.hpp"

#include <limits>

namespace VE
{
    Renderer::Renderer(const Instance &instance, const Surface &surface, const Window &window) : m_Surface(surface), m_Window(window)
    {
        m_Device = std::make_unique<Device>(instance, m_Surface);
        m_Swapchain = std::make_unique<Swapchain>(*m_Device, m_Surface, m_Window);

        m_RenderPass = std::make_unique<RenderPass>(*m_Swapchain);
        m_Pipeline = std::make_unique<GraphicsPipeline>(*m_Swapchain);

        CreateFramebuffers();

        m_CommandPool = std::make_unique<CommandPool>(*m_Device);
        CreateCommandBuffers();

        CreateSyncObjects();
    }

    Renderer::~Renderer() = default;

    void Renderer::DrawFrame()
    {
        constexpr auto noTimeout = std::numeric_limits<uint64_t>::max();

        auto &inFlightFence = m_InFlightFences[currentFrame];
        const auto imageAvailableSemaphore = m_ImageAvailableSemaphores[currentFrame].Handle();
        const auto renderFinishedSemaphore = m_RenderFinishedSemaphores[currentFrame].Handle();

        inFlightFence.Wait(noTimeout);
        inFlightFence.Reset();

        uint32_t imageIndex;
        vkAcquireNextImageKHR(m_Device->Handle(), m_Swapchain->Handle(), noTimeout,
                              imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);

        auto commandBuffer = m_CommandBuffers->Begin(imageIndex);
        { // Render
            VkRenderPassBeginInfo renderPassInfo{};
            renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            renderPassInfo.renderPass = m_RenderPass->Handle();
            renderPassInfo.framebuffer = m_Framebuffers[imageIndex].Handle();
            renderPassInfo.renderArea.offset = {0, 0};
            renderPassInfo.renderArea.extent = m_Swapchain->GetExtent();

            VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
            renderPassInfo.clearValueCount = 1;
            renderPassInfo.pClearValues = &clearColor;

            vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
            vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipeline->Handle());

            vkCmdDraw(commandBuffer, 3, 1, 0, 0);
            vkCmdEndRenderPass(commandBuffer);
        }
        m_CommandBuffers->End(imageIndex);

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        VkSemaphore waitSemaphores[] = {imageAvailableSemaphore};
        VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;

        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;

        VkSemaphore signalSemaphores[] = {renderFinishedSemaphore};
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;

        CheckVk(vkQueueSubmit(m_Device->GraphicsQueue(), 1, &submitInfo, inFlightFence.Handle()), "submit draw command buffer!");

        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores;
        presentInfo.swapchainCount = 1;
        VkSwapchainKHR swapChains[] = {m_Swapchain->Handle()};
        presentInfo.pSwapchains = swapChains;
        presentInfo.pImageIndices = &imageIndex;

        vkQueuePresentKHR(m_Device->PresentQueue(), &presentInfo);

        currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
    }

    void Renderer::CreateFramebuffers()
    {
        auto imageViews = m_Swapchain->GetImageViews();
        m_Framebuffers.reserve(imageViews.size());
        for (auto &imageView : imageViews)
        {
            m_Framebuffers.emplace_back(*m_Device, *m_RenderPass, imageView, m_Swapchain->GetExtent());
        }
    }

    void Renderer::CreateCommandBuffers()
    {
        m_CommandBuffers = std::make_unique<CommandBuffers>(
            *m_CommandPool,
            static_cast<uint32_t>(m_Swapchain->GetImageViews().size()));
    }

    void Renderer::CreateSyncObjects()
    {
        auto imageCount = m_Swapchain->GetImageViews().size();

        m_ImageAvailableSemaphores.reserve(MAX_FRAMES_IN_FLIGHT);
        m_InFlightFences.reserve(MAX_FRAMES_IN_FLIGHT);
        m_RenderFinishedSemaphores.reserve(imageCount);

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
        {
            m_ImageAvailableSemaphores.emplace_back(*m_Device);
            m_InFlightFences.emplace_back(*m_Device, true);
        }

        for (size_t i = 0; i < imageCount; i++)
        {
            m_RenderFinishedSemaphores.emplace_back(*m_Device);
        }
    }
