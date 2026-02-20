#include "Renderer.hpp"
#include "Window.hpp"
#include "Instance.hpp"
#include "Surface.hpp"
#include "Device.hpp"
#include "Swapchain.hpp"
#include "RenderPass.hpp"
#include "DescriptorSetLayout.hpp"
#include "PipelineLayout.hpp"
#include "GraphicsPipeline.hpp"
#include "CommandPool.hpp"
#include "CommandBuffers.hpp"
#include "DepthBuffer.hpp"
#include "Validation.hpp"
// TODO: remove
#include "Buffer.hpp"
#include "DeviceMemory.hpp"
#include "DescriptorPool.hpp"
#include "DescriptorSets.hpp"
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <stb_image.h>
#include "SingleTimeCommands.hpp"
#include <chrono>
#include "Texture.hpp"
#include "TextureImage.hpp"
#include "ImageView.hpp"
#include "Sampler.hpp"
// ---

#include <limits>
#include <stdexcept>
#include <cstdint>
#include <map>

namespace VE
{
    Renderer::Renderer(const Instance &instance, const Surface &surface, const Window &window)
        : m_Window(window), m_Surface(surface)
    {
        m_Device = std::make_unique<Device>(instance, m_Surface);

        CreateSwapchain();
        CreateRenderPass();
        CreateDescriptorSetLayout();
        CreateGraphicsPipeline();
        CreateCommandPool();
        CreateDepthBuffer();
        CreateFramebuffers();

        // TODO: remove
        CreateTextureImage();
        CreateVertexBuffer();
        CreateIndexBuffer();
        CreateUniformBuffers();
        // ---

        CreateDescriptorPool();
        CreateDescriptorSets();

        CreateCommandBuffers();
        CreateSyncObjects();
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

        UpdateUniformBuffer(currentFrame);

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
                vkCmdBindIndexBuffer(commandBuffer, m_IndexBuffer->Handle(), 0, VK_INDEX_TYPE_UINT16);

                VkDescriptorSet descriptorSet = (*m_DescriptorSets)[currentFrame];
                vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                                        m_Pipeline->Layout().Handle(), 0, 1, &descriptorSet, 0, nullptr);

                vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(m_Indices.size()), 1, 0, 0, 0);
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

    void Renderer::CreateRenderPass()
    {
        m_RenderPass = std::make_unique<RenderPass>(*m_Swapchain);
    }

    void Renderer::CreateDescriptorSetLayout()
    {
        DescriptorBinding ubo;
        ubo.Binding = 0;
        ubo.DescriptorCount = 1;
        ubo.Stage = VK_SHADER_STAGE_VERTEX_BIT;
        ubo.Type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

        DescriptorBinding sampler;
        sampler.Binding = 1;
        sampler.DescriptorCount = 1;
        sampler.Stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        sampler.Type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

        m_DescriptorBindings = std::vector<DescriptorBinding>{ubo, sampler};

        m_DescriptorSetLayout = std::make_unique<DescriptorSetLayout>(*m_Device, m_DescriptorBindings);
    }

    void Renderer::CreateGraphicsPipeline()
    {
        m_Pipeline = std::make_unique<GraphicsPipeline>(*m_Swapchain, *m_RenderPass, *m_DescriptorSetLayout);
    }

    void Renderer::CreateCommandPool()
    {
        m_CommandPool = std::make_unique<CommandPool>(*m_Device);
    }

    void Renderer::CreateDepthBuffer()
    {
        m_DepthBuffer = std::make_unique<DepthBuffer>(*m_CommandPool, m_Swapchain->GetExtent());
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

    void Renderer::CreateTextureImage()
    {
        Texture texture = Texture::LoadTexture("textures/texture.jpg");
        m_TextureImage = std::make_unique<TextureImage>(*m_CommandPool, texture);
    }

    void Renderer::CreateVertexBuffer()
    {
        m_Vertices = std::vector<Vertex>{
            {{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
            {{0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
            {{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
            {{-0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}},

            {{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
            {{0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
            {{0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
            {{-0.5f, 0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}}};

        auto [vertexBuffer, vertexMemory] = Buffer::CreateFromData(*m_CommandPool, m_Vertices, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
        m_VertexBuffer = std::move(vertexBuffer);
        m_VertexBufferMemory = std::move(vertexMemory);
    }

    void Renderer::CreateIndexBuffer()
    {
        m_Indices = std::vector<uint16_t>{
            0, 1, 2, 2, 3, 0,
            4, 5, 6, 6, 7, 4};

        auto [indexBuffer, indexMemory] = Buffer::CreateFromData(*m_CommandPool, m_Indices, VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
        m_IndexBuffer = std::move(indexBuffer);
        m_IndexBufferMemory = std::move(indexMemory);
    }

    void Renderer::CreateUniformBuffers()
    {
        m_UniformBuffers.clear();
        m_UniformBuffers.reserve(MAX_FRAMES_IN_FLIGHT);

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
        {
            m_UniformBuffers.emplace_back(*m_Device);
        }
    }

    void Renderer::CreateDescriptorPool()
    {
        m_DescriptorPool = std::make_unique<DescriptorPool>(*m_Device, m_DescriptorBindings, MAX_FRAMES_IN_FLIGHT);
    }

    void Renderer::CreateDescriptorSets()
    {
        std::map<uint32_t, VkDescriptorType> bindings = {
            {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER},
            {1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER}};

        m_DescriptorSets = std::make_unique<DescriptorSets>(*m_DescriptorPool, *m_DescriptorSetLayout, bindings, MAX_FRAMES_IN_FLIGHT);

        std::vector<VkWriteDescriptorSet> descriptorWrites;

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
        {
            VkDescriptorBufferInfo bufferInfo{};
            bufferInfo.buffer = m_UniformBuffers[i].GetBuffer().Handle();
            bufferInfo.offset = 0;
            bufferInfo.range = sizeof(UniformBufferObject);

            descriptorWrites.push_back(m_DescriptorSets->Bind(i, 0, bufferInfo, 1));

            VkDescriptorImageInfo imageInfo{};
            imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imageInfo.imageView = m_TextureImage->GetImageView().Handle();
            imageInfo.sampler = m_TextureImage->GetSampler().Handle();

            descriptorWrites.push_back(m_DescriptorSets->Bind(i, 1, imageInfo, 1));
        }

        m_DescriptorSets->UpdateDescriptors(descriptorWrites);
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
        
        m_Framebuffers.clear();
        // m_DepthBuffer.reset();
        m_Pipeline.reset();
        m_RenderPass.reset();
        m_Swapchain.reset();

        CreateSwapchain();
        CreateRenderPass();
        CreateGraphicsPipeline();
        // CreateDepthBuffer();
        CreateFramebuffers();
    }

    void Renderer::UpdateUniformBuffer(uint32_t currentImage)
    {
        static auto startTime = std::chrono::high_resolution_clock::now();

        auto currentTime = std::chrono::high_resolution_clock::now();
        float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

        VkExtent2D swapchainExtent = m_Swapchain->GetExtent();

        UniformBufferObject ubo{};
        ubo.Model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        ubo.View = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f),
                               glm::vec3(0.0f, 0.0f, 0.0f),
                               glm::vec3(0.0f, 0.0f, 1.0f));
        ubo.Proj = glm::perspective(glm::radians(45.0f), swapchainExtent.width / (float)swapchainExtent.height, 0.1f, 10.0f);
        ubo.Proj[1][1] *= -1;

        m_UniformBuffers[currentImage].SetValue(ubo);
    }

}
