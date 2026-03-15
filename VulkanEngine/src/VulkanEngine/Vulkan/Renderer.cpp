#include "Renderer.hpp"
#include "VulkanEngine/Vulkan/Debug/VulkanValidation.hpp"
#include "VulkanEngine/Vulkan/Vertex.hpp"
#include "VulkanEngine/Core/Log.hpp"

#include <cassert>
#include <cstring> // std::memcpy

// TODO: remove
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <chrono>
#include <stdexcept>
// ---

static const std::vector<ve::Vertex> s_Vertices = {
    {{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}},
    {{0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}},
    {{0.5f, 0.5, 0.0f}, {0.0f, 0.0f, 1.0f}},
    {{-0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}},
};

static const std::vector<uint32_t> s_Indices = {0, 1, 2, 2, 3, 0};

namespace ve
{
    Renderer::Renderer(Ref<VulkanContext> context, VkExtent2D windowExtent)
        : m_Context(std::move(context)), m_Device(m_Context->GetDevice()), m_FramebufferExtent(windowExtent)
    {
        VE_CORE_TRACE("--- Initializing Renderer --------------");

        m_Swapchain = CreateScope<VulkanSwapchain>(
            m_Device, m_Context->GetSurface(),
            m_Context->GetPhysicalDevice(), m_FramebufferExtent);

        m_RenderPass = CreateScope<VulkanRenderPass>(m_Device, m_Swapchain->GetImageFormat(), "Graphics");

        CreateDescriptorSetManager();
        CreateGraphicsPipeline();

        CreateFramebuffers();

        CreateCommandPool();
        CreateCommandBuffers();

        CreateUniformBuffers();
        WriteDescriptorSets();

        CreateTextureImage();
        CreateVertexBuffer();
        CreateIndexBuffer();

        CreateSyncObjects();

        VE_CORE_INFO("Renderer initialized successfully");
    }

    Renderer::~Renderer()
    {
        VE_CORE_TRACE("--- Destroying Renderer ----------------");

        m_Context->DeviceWaitIdle();

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
        {
            vkDestroyFence(m_Device, m_InFlightFences[i], nullptr);
            vkDestroySemaphore(m_Device, m_RenderFinishedSemaphores[i], nullptr);
            vkDestroySemaphore(m_Device, m_ImageAvailableSemaphores[i], nullptr);
        }
        VE_CORE_TRACE("Destroyed {0} VkFence (in flight) objects", MAX_FRAMES_IN_FLIGHT);
        VE_CORE_TRACE("Destroyed {0} VkSemaphore (render finished) objects", MAX_FRAMES_IN_FLIGHT);
        VE_CORE_TRACE("Destroyed {0} VkSemaphore (image available) objects", MAX_FRAMES_IN_FLIGHT);

        VE_CORE_TRACE("Destroying Uniformbuffers({0})...", MAX_FRAMES_IN_FLIGHT);
        size_t uniformbuffersSize = m_UniformBuffers.size();
        m_UniformBuffersMemory.clear();
        m_UniformBuffers.clear();
        VE_CORE_TRACE("Destroyed {0} Uniformbuffers", uniformbuffersSize);

        m_IndexBufferMemory.reset();
        m_IndexBuffer.reset();

        m_VertexBufferMemory.reset();
        m_VertexBuffer.reset();

        m_CommandPool.reset();

        size_t framebuffersSize = m_Framebuffers.size();
        m_Framebuffers.clear();
        VE_CORE_TRACE("Destroyed {0} VkFramebuffer objects", framebuffersSize);

        m_GraphicsPipeline.reset();
        m_PipelineLayout.reset();
        m_DescriptorSetManager.reset();

        m_RenderPass.reset();

        m_Swapchain.reset();
    }

    void Renderer::DrawFrame()
    {
        constexpr uint64_t noTimeout = std::numeric_limits<uint64_t>::max();

        vkWaitForFences(m_Device, 1, &m_InFlightFences[m_CurrentFrame], VK_TRUE, noTimeout);

        uint32_t imageIndex;
        VkResult result = vkAcquireNextImageKHR(m_Device, m_Swapchain->GetSwapchain(), noTimeout,
                                                m_ImageAvailableSemaphores[m_CurrentFrame],
                                                VK_NULL_HANDLE, &imageIndex);
        if (result == VK_ERROR_OUT_OF_DATE_KHR)
        {
            RecreateSwapchain();
            return;
        }
        else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
        {
            throw std::runtime_error("failed to acquire swapchain image!");
        }

        vkResetFences(m_Device, 1, &m_InFlightFences[m_CurrentFrame]);

        vkResetCommandBuffer(m_CommandBuffers[m_CurrentFrame], 0);
        RecordCommandBuffer(m_CommandBuffers[m_CurrentFrame], imageIndex);

        UpdateUniformBuffers();

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        VkSemaphore waitSemaphores[] = {m_ImageAvailableSemaphores[m_CurrentFrame]};
        VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};

        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &m_CommandBuffers[m_CurrentFrame];

        VkSemaphore signalSemaphores[] = {m_RenderFinishedSemaphores[m_CurrentFrame]};
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;

        result = vkQueueSubmit(m_Context->GetGraphicsQueue(), 1, &submitInfo, m_InFlightFences[m_CurrentFrame]);
        CHECK_VK_RESULT(result);

        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores;

        VkSwapchainKHR swapchains[] = {m_Swapchain->GetSwapchain()};
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapchains;
        presentInfo.pImageIndices = &imageIndex;

        result = vkQueuePresentKHR(m_Context->GetPresentQueue(), &presentInfo);
        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_FramebufferResized)
        {
            m_FramebufferResized = false;
            RecreateSwapchain();
        }
        else if (result != VK_SUCCESS)
        {
            throw std::runtime_error("failed to present swapchain image!");
        }

        m_CurrentFrame = (m_CurrentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
    }

    void Renderer::OnWindowResize(uint32_t width, uint32_t height)
    {
        m_FramebufferResized = true;
        m_FramebufferExtent = {width, height};
    }

    void Renderer::CreateDescriptorSetManager()
    {
        DescriptorBinding uboBinding{
            .binding = 0,
            .descriptorCount = 1,
            .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .stage = VK_SHADER_STAGE_VERTEX_BIT};

        std::vector<DescriptorBinding> bindings = {uboBinding};
        m_DescriptorSetManager = CreateScope<DescriptorSetManager>(m_Device, bindings, MAX_FRAMES_IN_FLIGHT);
    }

    void Renderer::CreateGraphicsPipeline()
    {
        // Pipeline layout
        std::vector<VkDescriptorSetLayout> setLayouts = {
            m_DescriptorSetManager->GetDescriptorSetLayout().GetLayout(),
        };
        m_PipelineLayout = CreateScope<VulkanPipelineLayout>(m_Device, setLayouts, "Graphics");

        // Pipeline
        PipelineConfig pipelineConfig{};
        VulkanPipeline::DefaultPipelineConfig(pipelineConfig);

        pipelineConfig.bindingDescriptions = {Vertex::GetBindingDescription()};

        auto attributeDescriptions = Vertex::GetAttributeDescriptions();
        pipelineConfig.attributeDescriptions.assign(attributeDescriptions.begin(), attributeDescriptions.end());

        pipelineConfig.pipelineLayout = m_PipelineLayout->GetPipelineLayout();
        pipelineConfig.renderPass = m_RenderPass->GetRenderPass();

        m_GraphicsPipeline = CreateScope<VulkanPipeline>(
            m_Device,
            "../VulkanEngine/assets/shaders/shader.vert.spv",
            "../VulkanEngine/assets/shaders/shader.frag.spv",
            pipelineConfig, "Graphics");
    }

    void Renderer::CreateFramebuffers()
    {
        uint32_t imageCount = m_Swapchain->GetImageCount();

        m_Framebuffers.resize(imageCount);
        for (uint32_t i = 0; i < imageCount; i++)
        {
            std::vector<VkImageView> attachments = {m_Swapchain->GetImageView(i)};
            m_Framebuffers[i] = CreateScope<VulkanFramebuffer>(
                m_Device, m_RenderPass->GetRenderPass(),
                m_Swapchain->GetExtent(),
                attachments);
        }
        VE_CORE_TRACE("Created {0} VkFramebuffer objects", m_Framebuffers.size());
    }

    void Renderer::CreateCommandPool()
    {
        auto queueIndices = m_Context->GetPhysicalDevice().GetQueueIndices();
        m_CommandPool = CreateScope<VulkanCommandPool>(m_Device, queueIndices.GraphicsFamily.value());
    }

    void Renderer::CreateCommandBuffers()
    {
        m_CommandBuffers.resize(MAX_FRAMES_IN_FLIGHT);

        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = m_CommandPool->GetCommandPool();
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = static_cast<uint32_t>(m_CommandBuffers.size());

        VkResult result = vkAllocateCommandBuffers(m_Device, &allocInfo, m_CommandBuffers.data());
        CHECK_VK_RESULT(result);
        VE_CORE_TRACE("VkCommandBuffer created");
    }

    void Renderer::RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex)
    {
        VkExtent2D swapchainExtent = m_Swapchain->GetExtent();

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = 0;                  // Optional
        beginInfo.pInheritanceInfo = nullptr; // Optional

        VkResult result = vkBeginCommandBuffer(commandBuffer, &beginInfo);
        CHECK_VK_RESULT(result);

        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = m_RenderPass->GetRenderPass();
        renderPassInfo.framebuffer = m_Framebuffers[imageIndex]->GetFramebuffer();
        renderPassInfo.renderArea.offset = {0, 0};
        renderPassInfo.renderArea.extent = swapchainExtent;

        VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
        renderPassInfo.clearValueCount = 1;
        renderPassInfo.pClearValues = &clearColor;

        vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        m_GraphicsPipeline->Bind(commandBuffer);

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(swapchainExtent.width);
        viewport.height = static_cast<float>(swapchainExtent.height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

        VkRect2D scissor{};
        scissor.offset = {0, 0};
        scissor.extent = swapchainExtent;
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

        VkBuffer vertexBuffers[] = {m_VertexBuffer->GetBuffer()};
        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
        vkCmdBindIndexBuffer(commandBuffer, m_IndexBuffer->GetBuffer(), 0, VK_INDEX_TYPE_UINT32);

        VkDescriptorSet set = m_DescriptorSetManager->GetSets()[m_CurrentFrame];
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                                m_PipelineLayout->GetPipelineLayout(), 0, 1, &set, 0, nullptr);

        vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(s_Indices.size()), 1, 0, 0, 0);

        vkCmdEndRenderPass(commandBuffer);

        result = vkEndCommandBuffer(commandBuffer);
        CHECK_VK_RESULT(result);
    }

    void Renderer::CreateSyncObjects()
    {
        m_ImageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        m_RenderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        m_InFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        VkResult result;
        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
        {
            result = vkCreateSemaphore(m_Device, &semaphoreInfo, nullptr, &m_ImageAvailableSemaphores[i]);
            CHECK_VK_RESULT(result);

            result = vkCreateSemaphore(m_Device, &semaphoreInfo, nullptr, &m_RenderFinishedSemaphores[i]);
            CHECK_VK_RESULT(result);

            result = vkCreateFence(m_Device, &fenceInfo, nullptr, &m_InFlightFences[i]);
            CHECK_VK_RESULT(result);
        }

        VE_CORE_TRACE("Created {0} VkSemaphore (image available) objects", MAX_FRAMES_IN_FLIGHT);
        VE_CORE_TRACE("Created {0} VkSemaphore (render finished) objects", MAX_FRAMES_IN_FLIGHT);
        VE_CORE_TRACE("Created {0} VkFence (in flight) objects", MAX_FRAMES_IN_FLIGHT);
    }

    void Renderer::CreateTextureImage()
    {
        int texWidth, texHeight, texChannels;
        stbi_uc *pixels = stbi_load("../VulkanEngine/assets/textures/texture.jpg", &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
        VkDeviceSize imageSize = texWidth * texHeight * 4;

        if (!pixels)
            throw std::runtime_error("failed to load texture image!");

        VkPhysicalDevice physicalDevice = m_Context->GetPhysicalDevice().GetPhysicalDevice();
        VulkanBuffer stagingBuffer(m_Device, imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, "TextureImage staging");
        VulkanDeviceMemory stagingMemory = stagingBuffer.AllocateMemory(
            physicalDevice, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        void *mapped = stagingMemory.Map(imageSize);
        std::memcpy(mapped, pixels, imageSize);
        stagingMemory.Unmap();
        stbi_image_free(pixels);

        m_TextureImage = CreateScope<VulkanImage>(
            m_Device, VkExtent2D{static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight)},
            VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL,
            VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, "TextureImage");
        m_TextureImageMemory = CreateScope<VulkanDeviceMemory>(m_TextureImage->AllocateMemory(
            physicalDevice, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT));

        VkCommandPool commandPool = m_CommandPool->GetCommandPool();
        VkQueue graphicsQueue = m_Context->GetGraphicsQueue();
        m_TextureImage->TransitionImageLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, commandPool, graphicsQueue);
        m_TextureImage->CopyFrom(stagingBuffer.GetBuffer(), commandPool, graphicsQueue);
        m_TextureImage->TransitionImageLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, commandPool, graphicsQueue);
    }

    void Renderer::CreateVertexBuffer()
    {
        VkPhysicalDevice physicalDevice = m_Context->GetPhysicalDevice().GetPhysicalDevice();
        VkDeviceSize bufferSize = sizeof(Vertex) * s_Vertices.size();

        VulkanBuffer stagingBuffer(m_Device, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, "VertexBuffer staging");
        VulkanDeviceMemory stagingMemory = stagingBuffer.AllocateMemory(
            physicalDevice, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        void *mapped = stagingMemory.Map(bufferSize);
        std::memcpy(mapped, s_Vertices.data(), bufferSize);
        stagingMemory.Unmap();

        m_VertexBuffer = CreateScope<VulkanBuffer>(
            m_Device, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, "VertexBuffer");
        m_VertexBufferMemory = CreateScope<VulkanDeviceMemory>(m_VertexBuffer->AllocateMemory(
            physicalDevice, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT));

        m_VertexBuffer->CopyFrom(stagingBuffer.GetBuffer(), bufferSize, m_CommandPool->GetCommandPool(), m_Context->GetGraphicsQueue());
    }

    void Renderer::CreateIndexBuffer()
    {
        VkPhysicalDevice physicalDevice = m_Context->GetPhysicalDevice().GetPhysicalDevice();
        VkDeviceSize bufferSize = sizeof(uint32_t) * s_Indices.size();

        VulkanBuffer stagingBuffer(m_Device, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, "IndexBuffer staging");
        VulkanDeviceMemory stagingMemory = stagingBuffer.AllocateMemory(
            physicalDevice, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        void *mapped = stagingMemory.Map(bufferSize);
        std::memcpy(mapped, s_Indices.data(), bufferSize);
        stagingMemory.Unmap();

        m_IndexBuffer = CreateScope<VulkanBuffer>(
            m_Device, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, "IndexBuffer");
        m_IndexBufferMemory = CreateScope<VulkanDeviceMemory>(m_IndexBuffer->AllocateMemory(
            physicalDevice, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT));

        m_IndexBuffer->CopyFrom(stagingBuffer.GetBuffer(), bufferSize, m_CommandPool->GetCommandPool(), m_Context->GetGraphicsQueue());
    }

    void Renderer::CreateUniformBuffers()
    {
        VkDeviceSize bufferSize = sizeof(UniformBufferObject);

        m_UniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);
        m_UniformBuffersMemory.resize(MAX_FRAMES_IN_FLIGHT);

        VE_CORE_TRACE("Creating UniformBuffers({0})...", MAX_FRAMES_IN_FLIGHT);
        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
        {
            VkPhysicalDevice physicalDevice = m_Context->GetPhysicalDevice().GetPhysicalDevice();
            m_UniformBuffers[i] = CreateScope<VulkanBuffer>(
                m_Device, bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                "UniformBuffer " + std::to_string(i));

            m_UniformBuffersMemory[i] = CreateScope<VulkanDeviceMemory>(m_UniformBuffers[i]->AllocateMemory(
                physicalDevice, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT));
        }

        VE_CORE_TRACE("Created {0} UniformBuffers", m_UniformBuffers.size());
    }

    void Renderer::WriteDescriptorSets()
    {
        std::vector<VkWriteDescriptorSet> writes;
        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
        {
            VkDescriptorBufferInfo bufferInfo{};
            bufferInfo.buffer = m_UniformBuffers[i]->GetBuffer();
            bufferInfo.range = sizeof(UniformBufferObject);

            writes.push_back(m_DescriptorSetManager->GetSets().Bind(i, 0, bufferInfo));
        }

        m_DescriptorSetManager->GetSets().UpdateDescriptors(writes);
    }

    void Renderer::RecreateSwapchain()
    {
        assert(m_FramebufferExtent.width != 0 || m_FramebufferExtent.height != 0);

        VE_CORE_INFO("Recreating swapchain for extent: {0}x{1}...", m_FramebufferExtent.width, m_FramebufferExtent.height);

        m_Context->DeviceWaitIdle();

        uint32_t framebuffersSize = static_cast<uint32_t>(m_Framebuffers.size());
        m_Framebuffers.clear();
        VE_CORE_TRACE("Destroyed {0} VkFramebuffer objects", framebuffersSize);
        m_Swapchain.reset();

        m_Swapchain = CreateScope<VulkanSwapchain>(m_Device, m_Context->GetSurface(), m_Context->GetPhysicalDevice(), m_FramebufferExtent);
        CreateFramebuffers();

        VE_CORE_INFO("Swapchain recreated successfully.");
    }

    void Renderer::UpdateUniformBuffers()
    {
        static auto startTime = std::chrono::high_resolution_clock::now();

        auto currentTime = std::chrono::high_resolution_clock::now();
        float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

        UniformBufferObject ubo{};
        ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        ubo.proj = glm::perspective(glm::radians(45.0f), m_Swapchain->GetExtent().width / (float)m_Swapchain->GetExtent().height, 0.1f, 10.0f);
        ubo.proj[1][1] *= -1;

        auto &uniformBufferMemory = m_UniformBuffersMemory[m_CurrentFrame];
        void *mapped = uniformBufferMemory->Map(sizeof(UniformBufferObject));
        std::memcpy(mapped, &ubo, sizeof(UniformBufferObject));
        uniformBufferMemory->Unmap();
    }

} // namespace ve
