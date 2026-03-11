#include "VulkanCore.hpp"
#include "VulkanEngine/Core/Base.hpp"
#include "VulkanEngine/Core/Log.hpp"
#include "Debug/VulkanValidation.hpp"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <set>
#include <limits>
#include <vector>

static const std::vector<ve::Vertex> s_Vertices = {
    {{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}},
    {{0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}},
    {{0.5f, 0.5, 0.0f}, {0.0f, 0.0f, 1.0f}},
    {{-0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}},
};

static const std::vector<uint32_t> s_Indices = {0, 1, 2, 2, 3, 0};

namespace ve
{
    namespace
    {
        VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
            VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
            VkDebugUtilsMessageTypeFlagsEXT messageType,
            const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
            void *pUserData)
        {
            if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT)
            {
                VE_CORE_TRACE("Vulkan Validation:\n{0}", pCallbackData->pMessage);
            }
            else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)
            {
                VE_CORE_INFO("Vulkan Validation:\n{0}", pCallbackData->pMessage);
            }
            else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
            {
                VE_CORE_WARN("Vulkan Validation:\n{0}", pCallbackData->pMessage);
            }
            else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
            {
                VE_CORE_ERROR("Vulkan Validation:\n{0}", pCallbackData->pMessage);
            }
            return VK_FALSE;
        }

        VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkDebugUtilsMessengerEXT *pCallback)
        {
            const auto func = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT"));
            return func != nullptr
                       ? func(instance, pCreateInfo, pAllocator, pCallback)
                       : VK_ERROR_EXTENSION_NOT_PRESENT;
        }

        void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT callback, const VkAllocationCallbacks *pAllocator)
        {
            const auto func = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT"));
            if (func != nullptr)
            {
                func(instance, callback, pAllocator);
            }
        }
    }

    VulkanCore::VulkanCore()
    {
    }

    VulkanCore::~VulkanCore()
    {
        VE_CORE_TRACE("----------------------------------------");

        if (m_Device != VK_NULL_HANDLE)
            vkDeviceWaitIdle(m_Device);

        // TODO: remove
        m_IndexBuffer.reset();
        m_VertexBuffer.reset();
        // ---

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
        {
            vkDestroyFence(m_Device, m_InFlightFences[i], nullptr);
            vkDestroySemaphore(m_Device, m_RenderFinishedSemaphores[i], nullptr);
            vkDestroySemaphore(m_Device, m_ImageAvailableSemaphores[i], nullptr);
        }
        VE_CORE_TRACE("Created {0} VkFence (in flight) objects", MAX_FRAMES_IN_FLIGHT);
        VE_CORE_TRACE("Created {0} VkSemaphore (render finished) objects", MAX_FRAMES_IN_FLIGHT);
        VE_CORE_TRACE("Created {0} VkSemaphore (image available) objects", MAX_FRAMES_IN_FLIGHT);

        m_CommandPool.reset();

        uint32_t framebuffersSize = static_cast<uint32_t>(m_Framebuffers.size());
        m_Framebuffers.clear();
        VE_CORE_TRACE("Destroyed {0} VkFramebuffer objects", framebuffersSize);

        m_Pipeline.reset();

        m_PipelineLayout.reset();

        m_RenderPass.reset();

        m_Swapchain.reset();

        vkDestroyDevice(m_Device, nullptr);
        VE_CORE_TRACE("VkDevice destroyed");

        vkDestroySurfaceKHR(m_Instance, m_Surface, nullptr);
        VE_CORE_TRACE("VkSurfaceKHR destroyed");

        if (m_DebugMessenger != VK_NULL_HANDLE)
        {
            DestroyDebugUtilsMessengerEXT(m_Instance, m_DebugMessenger, nullptr);
            VE_CORE_TRACE("VkDebugUtilsMessengerEXT destroyed");
        }

        vkDestroyInstance(m_Instance, nullptr);
        VE_CORE_TRACE("VkInstance destroyed");
    }

    void VulkanCore::Init(const VulkanConfig &config, GLFWwindow *window)
    {
        VE_CORE_TRACE("Initializing VulkanCore...");
        CreateInstance(config);
        CreateDebugCallback(config);
        CreateSurface(window);

        // Physical device
        PhysicalDeviceRequirements devReq;
        devReq.RequiresGraphicsQueue = true;
        devReq.RequiresPresentQueue = true;
        devReq.SwapchainAdequate = true;
        devReq.Extensions = {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME,
            VK_KHR_SHADER_DRAW_PARAMETERS_EXTENSION_NAME,
        };
        devReq.Features.geometryShader = VK_TRUE;
        devReq.Features.tessellationShader = VK_TRUE;
        devReq.PreferredDeviceType = VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
        m_PhysicalDevice = CreateScope<VulkanPhysicalDevice>(VulkanPhysicalDevice::Select(m_Instance, m_Surface, devReq));
        // ---

        CreateDevice(devReq);

        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        m_FramebufferExtent = VkExtent2D{static_cast<uint32_t>(width), static_cast<uint32_t>(height)};

        m_Swapchain = CreateScope<VulkanSwapchain>(m_Device, m_Surface, *m_PhysicalDevice, m_FramebufferExtent);

        m_RenderPass = CreateScope<VulkanRenderPass>(m_Device, m_Swapchain->GetImageFormat());

        m_PipelineLayout = CreateScope<VulkanPipelineLayout>(m_Device, "MainGraphics");

        CreateGraphicsPipeline();

        CreateFramebuffers();

        CreateCommandPool();

        // TODO: remove
        CreateVertexBuffer();
        CreateIndexBuffer();
        //

        CreateCommandBuffers();

        CreateSyncObjects();

        VE_CORE_INFO("VulkanCore initialized successfully");
    }

    void VulkanCore::DrawFrame()
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
            throw std::runtime_error("failed to acquire swap chainimage!");
        }

        vkResetFences(m_Device, 1, &m_InFlightFences[m_CurrentFrame]);

        vkResetCommandBuffer(m_CommandBuffers[m_CurrentFrame], 0);
        RecordCommandBuffer(m_CommandBuffers[m_CurrentFrame], imageIndex);

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

        result = vkQueueSubmit(m_GraphicsQueue, 1, &submitInfo, m_InFlightFences[m_CurrentFrame]);
        CHECK_VK_RESULT(result);

        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores;

        VkSwapchainKHR swapchains[] = {m_Swapchain->GetSwapchain()};
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapchains;
        presentInfo.pImageIndices = &imageIndex;

        result = vkQueuePresentKHR(m_PresentQueue, &presentInfo);
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

    void VulkanCore::OnWindowResize(uint32_t width, uint32_t height)
    {
        m_FramebufferResized = true;
        m_FramebufferExtent = {width, height};
    }

    void VulkanCore::CreateInstance(const VulkanConfig &config)
    {
        VE_CORE_TRACE("Instance extensions ({0}):", config.InstanceExtensions.size());
        for (auto extension : config.InstanceExtensions)
            VE_CORE_TRACE("\t{0}", extension);

        VE_CORE_TRACE("Enable validation layers: {0}", config.EnableValidationLayers);
        VE_CORE_TRACE("Validation layers ({0}):", config.ValidationLayers.size());
        for (auto layer : config.ValidationLayers)
            VE_CORE_TRACE("\t{0}", layer);

        VkApplicationInfo appInfo{};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = config.AppName.c_str();
        appInfo.applicationVersion = config.AppVersion;
        appInfo.pEngineName = config.EngineName.c_str();
        appInfo.engineVersion = config.EngineVersion;
        appInfo.apiVersion = config.ApiVersion;
        appInfo.pNext = nullptr;

        VkInstanceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;
        createInfo.enabledExtensionCount = static_cast<uint32_t>(config.InstanceExtensions.size());
        createInfo.ppEnabledExtensionNames = config.InstanceExtensions.data();

        uint32_t layerCount = config.EnableValidationLayers ? static_cast<uint32_t>(config.ValidationLayers.size()) : 0;
        createInfo.enabledLayerCount = layerCount;
        createInfo.ppEnabledLayerNames = layerCount > 0 ? config.ValidationLayers.data() : nullptr;

        VkResult result = vkCreateInstance(&createInfo, nullptr, &m_Instance);
        CHECK_VK_RESULT(result);

        VE_CORE_TRACE("VkInstance created");
    }

    void VulkanCore::CreateDebugCallback(const VulkanConfig &config)
    {
        if (!config.EnableValidationLayers || !config.DebugConfig.EnableDebugMessenger)
        {
            VE_CORE_TRACE("Debug messenger disabled");
            return;
        }

        VkDebugUtilsMessengerCreateInfoEXT createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        createInfo.messageSeverity = config.DebugConfig.MessageSeverity;
        createInfo.messageType = config.DebugConfig.MessageType;
        createInfo.pfnUserCallback = DebugCallback;
        createInfo.pUserData = nullptr;

        VkResult result = CreateDebugUtilsMessengerEXT(m_Instance, &createInfo, nullptr, &m_DebugMessenger);
        CHECK_VK_RESULT(result);

        VE_CORE_TRACE("VkDebugUtilsMessengerEXT created");
    }

    void VulkanCore::CreateSurface(GLFWwindow *window)
    {
        VkResult result = glfwCreateWindowSurface(m_Instance, window, nullptr, &m_Surface);
        CHECK_VK_RESULT(result);
        VE_CORE_TRACE("VkSurfaceKHR created");
    }

    void VulkanCore::CreateDevice(const PhysicalDeviceRequirements &requirements)
    {
        VE_CORE_TRACE("Device extensions ({0}):", requirements.Extensions.size());
        for (auto extension : requirements.Extensions)
            VE_CORE_TRACE("\t{0}", extension);

        auto queueIndices = m_PhysicalDevice->GetQueueIndices();

        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
        std::set<uint32_t> uniqueQueueFamilies;

        if (queueIndices.GraphicsFamily.has_value())
            uniqueQueueFamilies.insert(queueIndices.GraphicsFamily.value());
        if (queueIndices.PresentFamily.has_value())
            uniqueQueueFamilies.insert(queueIndices.PresentFamily.value());

        float queuePriority = 1.0f;
        for (uint32_t queueFamily : uniqueQueueFamilies)
        {
            VkDeviceQueueCreateInfo queueCreateInfo{};
            queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo.queueFamilyIndex = queueFamily;
            queueCreateInfo.queueCount = 1;
            queueCreateInfo.pQueuePriorities = &queuePriority;
            queueCreateInfos.push_back(queueCreateInfo);
        }

        VkDeviceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        createInfo.pQueueCreateInfos = queueCreateInfos.data();
        createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());

        createInfo.pEnabledFeatures = &requirements.Features;

        createInfo.enabledExtensionCount = static_cast<uint32_t>(requirements.Extensions.size());
        createInfo.ppEnabledExtensionNames = requirements.Extensions.data();

        VkResult result = vkCreateDevice(m_PhysicalDevice->GetPhysicalDevice(), &createInfo, nullptr, &m_Device);
        CHECK_VK_RESULT(result);

        vkGetDeviceQueue(m_Device, queueIndices.GraphicsFamily.value(), 0, &m_GraphicsQueue);
        vkGetDeviceQueue(m_Device, queueIndices.PresentFamily.value(), 0, &m_PresentQueue);

        VE_CORE_TRACE("VkDevice created");
    }

    void VulkanCore::CreateGraphicsPipeline()
    {
        PipelineConfig pipelineConfig{};
        VulkanPipeline::DefaultPipelineConfig(pipelineConfig);

        pipelineConfig.bindingDescriptions = {
            Vertex::GetBindingDescription(),
        };

        auto attributeDescriptions = Vertex::GetAttributeDescriptions();
        pipelineConfig.attributeDescriptions.assign(attributeDescriptions.begin(), attributeDescriptions.end());

        pipelineConfig.pipelineLayout = m_PipelineLayout->GetPipelineLayout();
        pipelineConfig.renderPass = m_RenderPass->GetRenderPass();
        m_Pipeline = CreateScope<VulkanPipeline>(
            m_Device,
            "../VulkanEngine/assets/shaders/shader.vert.spv",
            "../VulkanEngine/assets/shaders/shader.frag.spv",
            pipelineConfig, "MainGraphics");
    }

    void VulkanCore::CreateFramebuffers()
    {
        uint32_t imageCount = m_Swapchain->GetImageCount();
        m_Framebuffers.resize(imageCount);
        for (uint32_t i = 0; i < imageCount; i++)
        {
            std::vector<VkImageView> attachments = {m_Swapchain->GetImageView(i)};
            m_Framebuffers[i] = CreateScope<VulkanFramebuffer>(
                m_Device, m_RenderPass->GetRenderPass(),
                m_Swapchain->GetExtent(), attachments);
        }
        VE_CORE_TRACE("Created {0} VkFramebuffer objects", m_Framebuffers.size());
    }

    void VulkanCore::CreateCommandPool()
    {
        auto queueIndices = m_PhysicalDevice->GetQueueIndices();

        m_CommandPool = CreateScope<VulkanCommandPool>(m_Device, queueIndices.GraphicsFamily.value(), "MainCommandPool");
    }

    void VulkanCore::CreateCommandBuffers()
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

    void VulkanCore::RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex)
    {
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
        renderPassInfo.renderArea.extent = m_Swapchain->GetExtent();

        VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
        renderPassInfo.clearValueCount = 1;
        renderPassInfo.pClearValues = &clearColor;

        vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        m_Pipeline->Bind(commandBuffer);

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

        VkBuffer vertexBuffers[] = {m_VertexBuffer->GetBuffer()};
        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
        vkCmdBindIndexBuffer(commandBuffer, m_IndexBuffer->GetBuffer(), 0, VK_INDEX_TYPE_UINT32);

        vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(s_Indices.size()), 1, 0, 0, 0);
        // vkCmdDraw(commandBuffer, static_cast<uint32_t>(s_Vertices.size()), 1, 0, 0);

        vkCmdEndRenderPass(commandBuffer);

        result = vkEndCommandBuffer(commandBuffer);
        CHECK_VK_RESULT(result);
    }

    void VulkanCore::CreateSyncObjects()
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

    void VulkanCore::RecreateSwapchain()
    {
        assert(m_FramebufferExtent.width != 0 || m_FramebufferExtent.height != 0);

        VE_CORE_INFO("Recreating swapchain for extent: {0}x{1}...", m_FramebufferExtent.width, m_FramebufferExtent.height);

        vkDeviceWaitIdle(m_Device);

        uint32_t framebuffersSize = static_cast<uint32_t>(m_Framebuffers.size());
        m_Framebuffers.clear();
        VE_CORE_TRACE("Destroyed {0} VkFramebuffer objects", framebuffersSize);
        m_Swapchain.reset();

        m_Swapchain = CreateScope<VulkanSwapchain>(m_Device, m_Surface, *m_PhysicalDevice, m_FramebufferExtent);

        CreateFramebuffers();

        VE_CORE_INFO("Swapchain recreated successfully.");
    }

    void VulkanCore::CreateVertexBuffer()
    {
        VkDeviceSize bufferSize = sizeof(Vertex) * s_Vertices.size();

        VulkanBuffer stagingBuffer(
            m_Device, m_PhysicalDevice->GetPhysicalDevice(),
            bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        stagingBuffer.Map();
        stagingBuffer.WriteToBuffer(s_Vertices.data());
        stagingBuffer.Unmap();

        m_VertexBuffer = CreateScope<VulkanBuffer>(
            m_Device, m_PhysicalDevice->GetPhysicalDevice(),
            bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        CopyBuffer(stagingBuffer.GetBuffer(), m_VertexBuffer->GetBuffer(), bufferSize);
    }

    void VulkanCore::CreateIndexBuffer()
    {
        VkDeviceSize bufferSize = sizeof(uint32_t) * s_Indices.size();

        VulkanBuffer stagingBuffer(
            m_Device, m_PhysicalDevice->GetPhysicalDevice(),
            bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        stagingBuffer.Map();
        stagingBuffer.WriteToBuffer(s_Indices.data());
        stagingBuffer.Unmap();

        m_IndexBuffer = CreateScope<VulkanBuffer>(
            m_Device, m_PhysicalDevice->GetPhysicalDevice(),
            bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        CopyBuffer(stagingBuffer.GetBuffer(), m_IndexBuffer->GetBuffer(), bufferSize);
    }

    void VulkanCore::CopyBuffer(VkBuffer src, VkBuffer dst, VkDeviceSize size)
    {
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = m_CommandPool->GetCommandPool();
        allocInfo.commandBufferCount = 1;

        VkCommandBuffer commandBuffer;
        VkResult result = vkAllocateCommandBuffers(m_Device, &allocInfo, &commandBuffer);
        CHECK_VK_RESULT(result);

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        result = vkBeginCommandBuffer(commandBuffer, &beginInfo);
        CHECK_VK_RESULT(result);

        VkBufferCopy copyRegion{};
        copyRegion.size = size;

        vkCmdCopyBuffer(commandBuffer, src, dst, 1, &copyRegion);

        result = vkEndCommandBuffer(commandBuffer);
        CHECK_VK_RESULT(result);

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;

        result = vkQueueSubmit(m_GraphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
        CHECK_VK_RESULT(result);

        result = vkQueueWaitIdle(m_GraphicsQueue);
        CHECK_VK_RESULT(result);

        vkFreeCommandBuffers(m_Device, m_CommandPool->GetCommandPool(), 1, &commandBuffer);
    }

} // namespace ve
