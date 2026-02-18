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
// TODO: remove
#include "Buffer.hpp"
#include "DeviceMemory.hpp"
// ---

#include <limits>
#include <stdexcept>

namespace VE
{
    static void copyBuffer(const CommandPool &commandPool, const Buffer &srcBuffer, Buffer &dstBuffer, VkDeviceSize size)
    {
        CommandBuffers commandBuffers(commandPool, 1);

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        vkBeginCommandBuffer(commandBuffers[0], &beginInfo);

        VkBufferCopy copyRegion{};
        copyRegion.srcOffset = 0; // Optional
        copyRegion.dstOffset = 0; // Optional
        copyRegion.size = size;

        vkCmdCopyBuffer(commandBuffers[0], srcBuffer.Handle(), dstBuffer.Handle(), 1, &copyRegion);

        vkEndCommandBuffer(commandBuffers[0]);

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffers[0];

        const auto graphicsQueue = commandPool.GetDevice().GraphicsQueue();
        vkQueueSubmit(graphicsQueue, 1, &submitInfo, nullptr);
        vkQueueWaitIdle(graphicsQueue);
    }

    Renderer::Renderer(const Instance &instance, const Surface &surface, const Window &window) : m_Surface(surface), m_Window(window)
    {
        m_Device = std::make_unique<Device>(instance, m_Surface);
        CreateSwapchain();

        m_RenderPass = std::make_unique<RenderPass>(*m_Swapchain);
        m_Pipeline = std::make_unique<GraphicsPipeline>(*m_Swapchain, *m_RenderPass);

        CreateFramebuffers();

        m_CommandPool = std::make_unique<CommandPool>(*m_Device);
        CreateCommandBuffers();

        CreateSyncObjects();

        // TODO: remove
        m_Vertices = std::vector<Vertex>{
            {{0.0f, -0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}},
            {{0.5f, 0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}},
            {{-0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}},
        };

        auto [vertexBuffer, vertexMemory] = Buffer::CreateFromData(*m_CommandPool, m_Vertices, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
        m_VertexBuffer = std::move(vertexBuffer);
        m_VertexBufferMemory = std::move(vertexMemory);
        // ---
    }

    Renderer::~Renderer()
    {
        m_Device->WaitIdle();
    }

    void Renderer::DrawFrame()
    {
        constexpr auto noTimeout = std::numeric_limits<uint64_t>::max();

        auto &inFlightFence = m_InFlightFences[currentFrame];
        const auto imageAvailableSemaphore = m_ImageAvailableSemaphores[currentFrame].Handle();
        const auto renderFinishedSemaphore = m_RenderFinishedSemaphores[currentFrame].Handle();

        inFlightFence.Wait(noTimeout);

        uint32_t imageIndex;
        VkResult result = vkAcquireNextImageKHR(m_Device->Handle(), m_Swapchain->Handle(), noTimeout,
                                                imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);

        if (result == VK_ERROR_OUT_OF_DATE_KHR)
        {
            RecreateSwapchain();
            return;
        }
        else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
        {
            throw std::runtime_error("failed to acquire swap chain image!");
        }

        inFlightFence.Reset();

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
            {
                vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipeline->Handle());

                VkViewport viewport{};
                viewport.x = 0.0f;
                viewport.y = 0.0f;
                viewport.width = static_cast<float>(m_Swapchain->GetExtent().width);
                viewport.height = static_cast<float>(m_Swapchain->GetExtent().height);
                viewport.minDepth = 0.0f;
                viewport.maxDepth = 1.0f;
                vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

                VkRect2D scissor{};
                scissor.offset = {0, 0};
                scissor.extent = m_Swapchain->GetExtent();
                vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

                VkBuffer vertexBuffers[] = {m_VertexBuffer->Handle()};
                VkDeviceSize offsets[] = {0};
                vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

                vkCmdDraw(commandBuffer, static_cast<uint32_t>(m_Vertices.size()), 1, 0, 0);
            }
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

        result = vkQueuePresentKHR(m_Device->PresentQueue(), &presentInfo);
        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_FramebufferResized)
        {
            m_FramebufferResized = false;
            RecreateSwapchain();
        }
        else if (result != VK_SUCCESS)
        {
            throw std::runtime_error("failed to present swap chain image!");
        }

        currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
    }

    void Renderer::OnResize()
    {
        m_FramebufferResized = true;
    }

    void Renderer::CreateSwapchain()
    {
        m_Swapchain = std::make_unique<Swapchain>(*m_Device, m_Surface, m_Window);
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
        m_ImageAvailableSemaphores.reserve(MAX_FRAMES_IN_FLIGHT);
        m_RenderFinishedSemaphores.reserve(MAX_FRAMES_IN_FLIGHT);
        m_InFlightFences.reserve(MAX_FRAMES_IN_FLIGHT);

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
        {
            m_ImageAvailableSemaphores.emplace_back(*m_Device);
            m_RenderFinishedSemaphores.emplace_back(*m_Device);
            m_InFlightFences.emplace_back(*m_Device, true);
        }
    }

    void Renderer::RecreateSwapchain()
    {
        m_Device->WaitIdle();

        while (m_Window.IsMinimized())
        {
            m_Window.WaitForEvents();
        }

        m_CommandBuffers.reset();
        m_Framebuffers.clear();
        m_RenderPass.reset();
        m_Pipeline.reset();
        m_Swapchain.reset();

        CreateSwapchain();
        m_RenderPass = std::make_unique<RenderPass>(*m_Swapchain);
        m_Pipeline = std::make_unique<GraphicsPipeline>(*m_Swapchain, *m_RenderPass);
        CreateFramebuffers();
        CreateCommandBuffers();
    }

}
