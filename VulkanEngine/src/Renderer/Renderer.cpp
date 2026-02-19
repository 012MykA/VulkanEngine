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
// ---

#include <limits>
#include <stdexcept>
#include <cstdint>
#include <map>

namespace VE
{
    Renderer::Renderer(const Instance &instance, const Surface &surface, const Window &window) : m_Surface(surface), m_Window(window)
    {
        m_Device = std::make_unique<Device>(instance, m_Surface);
        CreateSwapchain();

        m_RenderPass = std::make_unique<RenderPass>(*m_Swapchain);
        CreateDescriptorSetLayout();
        m_Pipeline = std::make_unique<GraphicsPipeline>(*m_Swapchain, *m_RenderPass, *m_DescriptorSetLayout);

        CreateFramebuffers();

        m_CommandPool = std::make_unique<CommandPool>(*m_Device);
        CreateTextureImage();
        CreateCommandBuffers();

        CreateSyncObjects();

        // TODO: remove
        CreateVertexBuffer();
        CreateIndexBuffer();
        CreateUniformBuffers();
        m_DescriptorPool = std::make_unique<DescriptorPool>(*m_Device, m_DescriptorBindings, MAX_FRAMES_IN_FLIGHT);

        std::map<uint32_t, VkDescriptorType> bindings = {std::make_pair(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)};
        m_DescriptorSets = std::make_unique<DescriptorSets>(*m_DescriptorPool, *m_DescriptorSetLayout,
                                                            bindings, MAX_FRAMES_IN_FLIGHT);
        CreateDescriptorSets();
        // ---
    }

    Renderer::~Renderer()
    {
        m_Device->WaitIdle();
        vkDestroyImage(m_Device->Handle(), m_TextureImage, nullptr);
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
        m_Pipeline = std::make_unique<GraphicsPipeline>(*m_Swapchain, *m_RenderPass, *m_DescriptorSetLayout);
        CreateFramebuffers();
        CreateCommandBuffers();
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

    void Renderer::CreateDescriptorSetLayout()
    {
        DescriptorBinding binding;
        binding.Binding = 0;
        binding.DescriptorCount = 1;
        binding.Stage = VK_SHADER_STAGE_VERTEX_BIT;
        binding.Type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

        m_DescriptorBindings = std::vector<DescriptorBinding>{binding};

        m_DescriptorSetLayout = std::make_unique<DescriptorSetLayout>(*m_Device, m_DescriptorBindings);
    }

    void Renderer::CreateTextureImage()
    {
        int texWidth, texHeight, texChannels;
        stbi_uc *pixels = stbi_load("textures/texture.jpg", &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
        if (!pixels)
            throw std::runtime_error("failed to load texture image!");

        VkDeviceSize imageSize = texWidth * texHeight * 4;

        // ---
        Buffer stagingBuffer(*m_Device, imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
        DeviceMemory stagingMemory = stagingBuffer.AllocateMemory(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                                                  VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        void *data = stagingMemory.Map(0, imageSize);
        std::memcpy(data, pixels, imageSize);
        stagingMemory.Unmap();
        stbi_image_free(pixels);

        CreateImage(texWidth, texHeight, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL,
                    VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        TransitionImageLayout(m_TextureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
        CopyBufferToImage(
            stagingBuffer.Handle(), m_TextureImage,
            static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));
        TransitionImageLayout(m_TextureImage, VK_FORMAT_R8G8B8A8_SRGB,
                              VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    }

    void Renderer::CreateImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling,
                               VkImageUsageFlags usage, VkMemoryPropertyFlags properties)
    {
        VkImageCreateInfo imageInfo{};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.extent.width = width;
        imageInfo.extent.height = height;
        imageInfo.extent.depth = 1;
        imageInfo.mipLevels = 1;
        imageInfo.arrayLayers = 1;
        imageInfo.format = format;
        imageInfo.tiling = tiling;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage = usage;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageInfo.flags = 0; // Optional

        CheckVk(vkCreateImage(m_Device->Handle(), &imageInfo, nullptr, &m_TextureImage), "create image!");

        VkMemoryRequirements memRequirements;
        vkGetImageMemoryRequirements(m_Device->Handle(), m_TextureImage, &memRequirements);

        m_TextureImageMemory = std::make_unique<DeviceMemory>(
            *m_Device, memRequirements.size, memRequirements.memoryTypeBits, 0, properties);

        vkBindImageMemory(m_Device->Handle(), m_TextureImage, m_TextureImageMemory->Handle(), 0);
    }

    void Renderer::TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout)
    {
        auto action = [&](VkCommandBuffer commandBuffer)
        {
            VkImageMemoryBarrier barrier{};
            barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            barrier.oldLayout = oldLayout;
            barrier.newLayout = newLayout;
            barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.image = image;
            barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            barrier.subresourceRange.baseMipLevel = 0;
            barrier.subresourceRange.levelCount = 1;
            barrier.subresourceRange.baseArrayLayer = 0;
            barrier.subresourceRange.layerCount = 1;

            VkPipelineStageFlags sourceStage;
            VkPipelineStageFlags destinationStage;

            if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
            {
                barrier.srcAccessMask = 0;
                barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

                sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
                destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            }
            else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
            {
                barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

                sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
                destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            }
            else
            {
                throw std::runtime_error("unsupported layout transition!");
            }

            vkCmdPipelineBarrier(commandBuffer, sourceStage, destinationStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);
        };
        SingleTimeCommands::Submit(*m_CommandPool, action);
    }

    void Renderer::CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height)
    {
        auto action = [&](VkCommandBuffer commandBuffer)
        {
            VkBufferImageCopy region{};
            region.bufferOffset = 0;
            region.bufferRowLength = 0;
            region.bufferImageHeight = 0;

            region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            region.imageSubresource.mipLevel = 0;
            region.imageSubresource.baseArrayLayer = 0;
            region.imageSubresource.layerCount = 1;

            region.imageOffset = {0, 0, 0};
            region.imageExtent = {width, height, 1};

            vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
        };
        SingleTimeCommands::Submit(*m_CommandPool, action);
    }

    void Renderer::CreateVertexBuffer()
    {
        m_Vertices = std::vector<Vertex>{
            {{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}},
            {{0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}},
            {{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}},
            {{-0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}},
        };

        auto [vertexBuffer, vertexMemory] = Buffer::CreateFromData(*m_CommandPool, m_Vertices, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
        m_VertexBuffer = std::move(vertexBuffer);
        m_VertexBufferMemory = std::move(vertexMemory);
    }

    void Renderer::CreateIndexBuffer()
    {
        m_Indices = std::vector<uint16_t>{0, 1, 2, 2, 3, 0};

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

    void Renderer::CreateDescriptorSets()
    {
        std::vector<VkWriteDescriptorSet> descriptorWrites;

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
        {
            VkDescriptorBufferInfo bufferInfo{};
            bufferInfo.buffer = m_UniformBuffers[i].GetBuffer().Handle();
            bufferInfo.offset = 0;
            bufferInfo.range = sizeof(UniformBufferObject);

            descriptorWrites.push_back(
                m_DescriptorSets->Bind(i, 0, bufferInfo, 1));
        }

        m_DescriptorSets->UpdateDescriptors(descriptorWrites);
    }

}
