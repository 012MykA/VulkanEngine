#include "VulkanCore.hpp"
#include "VulkanEngine/Core/Base.hpp"
#include "VulkanEngine/Core/Log.hpp"
#include "Debug/VulkanValidation.hpp"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <set>

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
        m_Swapchain = CreateScope<VulkanSwapchain>(
            m_Device, m_Surface, *m_PhysicalDevice,
            VkExtent2D{static_cast<uint32_t>(width), static_cast<uint32_t>(height)});

        m_RenderPass = CreateScope<VulkanRenderPass>(m_Device, m_Swapchain->GetImageFormat());

        m_PipelineLayout = CreateScope<VulkanPipelineLayout>(m_Device, "MainGraphics");

        PipelineConfig pipelineConfig{};
        VulkanPipeline::DefaultPipelineConfig(pipelineConfig);
        pipelineConfig.pipelineLayout = m_PipelineLayout->GetPipelineLayout();
        pipelineConfig.renderPass = m_RenderPass->GetRenderPass();
        m_Pipeline = CreateScope<VulkanPipeline>(
            m_Device,
            "../VulkanEngine/assets/shaders/shader.vert.spv",
            "../VulkanEngine/assets/shaders/shader.frag.spv",
            pipelineConfig, "MainGraphics");

        CreateFramebuffers();

        auto queueIndices = m_PhysicalDevice->GetQueueIndices();
        m_CommandPool = CreateScope<VulkanCommandPool>(m_Device, queueIndices.GraphicsFamily.value(), "MainCommandPool");

        CreateCommandBuffer();

        VE_CORE_INFO("VulkanCore initialized successfully");
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

    void VulkanCore::CreateCommandBuffer()
    {
    }

} // namespace ve
