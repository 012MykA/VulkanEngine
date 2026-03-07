#include "VulkanCore.hpp"
#include "VulkanEngine/Core/Base.hpp"
#include "VulkanEngine/Core/Log.hpp"
#include "Debug/Validation.hpp"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <set>
#include <algorithm>
#include <limits>

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
                VE_CORE_TRACE("Vulkan Validation: {0}\n", pCallbackData->pMessage);
            }
            else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)
            {
                VE_CORE_INFO("Vulkan Validation: {0}\n", pCallbackData->pMessage);
            }
            else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
            {
                VE_CORE_WARN("Vulkan Validation: {0}\n", pCallbackData->pMessage);
            }
            else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
            {
                VE_CORE_ERROR("Vulkan Validation: {0}\n", pCallbackData->pMessage);
            }
            return VK_FALSE;
        }

        VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkDebugUtilsMessengerEXT *pCallback)
        {
            const auto func = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT"));
            return func != nullptr ? func(instance, pCreateInfo, pAllocator, pCallback) : VK_ERROR_EXTENSION_NOT_PRESENT;
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
        VE_CORE_TRACE("---------------------------------------");

        vkDestroyCommandPool(m_Device, m_CommandPool, nullptr);
        VE_CORE_TRACE("VkCommandPool destroyed");

        for (auto imageView : m_SwapchainImageViews)
        {
            vkDestroyImageView(m_Device, imageView, nullptr);
        }
        VE_CORE_TRACE("Destroyed {0} swapchain VkImageView", m_SwapchainImageViews.size());

        vkDestroySwapchainKHR(m_Device, m_Swapchain, nullptr);
        VE_CORE_TRACE("VkSwapchainKHR destroyed");

        vkDestroyDevice(m_Device, nullptr);
        VE_CORE_TRACE("VkDevice destroyed");

        vkDestroySurfaceKHR(m_Instance, m_Surface, nullptr);
        VE_CORE_TRACE("VkSurfaceKHR destroyed");

        DestroyDebugUtilsMessengerEXT(m_Instance, m_DebugMessenger, nullptr);
        VE_CORE_TRACE("VkDebugUtilsMessengerEXT destroyed");

        vkDestroyInstance(m_Instance, nullptr);
        VE_CORE_TRACE("VkInstance destroyed");
    }

    void VulkanCore::Init(const VulkanConfig &config, GLFWwindow *window)
    {
        m_Window = window;
        VE_CORE_INFO("Initializing VulkanCore...");
        CreateInstance(config);
        CreateDebugCallback(config);
        CreateSurface(window);

        m_PhysicalDevices.Init(m_Instance);
        PhysicalDeviceRequirements deviceRequirements;
        deviceRequirements.Extensions = {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME,
            VK_KHR_SHADER_DRAW_PARAMETERS_EXTENSION_NAME,
        };
        deviceRequirements.Features.geometryShader = VK_TRUE;
        deviceRequirements.Features.tessellationShader = VK_TRUE;

        m_PhysicalDevices.SelectDevice(m_Surface, deviceRequirements);

        CreateDevice(deviceRequirements);
        CreateSwapchain();
        CreateImageViews();
        CreateCommandPool();

        VE_CORE_INFO("VulkanCore initialized successfully");
    }

    void VulkanCore::CreateCommandBuffers(uint32_t count, VkCommandBuffer *commandBuffers)
    {
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = m_CommandPool;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = count;

        VkResult result = vkAllocateCommandBuffers(m_Device, &allocInfo, commandBuffers);
        CHECK_VK_RESULT(result);
        VE_CORE_TRACE("Allocated {0} command buffers", count);
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
            VE_CORE_TRACE("Debug messenger is disabled");
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
        const auto &physicalDevice = m_PhysicalDevices.Selected();
        PhysicalDeviceQueueFamilyIndices queueIndices = m_PhysicalDevices.GetQueueIndices(m_Surface);

        // Create queue create infos
        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
        std::set<uint32_t> uniqueQueueFamilies;

        if (requirements.RequiresGraphicsQueue && queueIndices.GraphicsFamily.has_value())
            uniqueQueueFamilies.insert(queueIndices.GraphicsFamily.value());
        if (requirements.RequiresPresentQueue && queueIndices.PresentFamily.has_value())
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

        // Create device
        VkDeviceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        createInfo.pQueueCreateInfos = queueCreateInfos.data();
        createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
        createInfo.pEnabledFeatures = &requirements.Features;
        createInfo.enabledExtensionCount = static_cast<uint32_t>(requirements.Extensions.size());
        createInfo.ppEnabledExtensionNames = requirements.Extensions.empty() ? nullptr : requirements.Extensions.data();

        VkResult result = vkCreateDevice(physicalDevice.Device, &createInfo, nullptr, &m_Device);
        CHECK_VK_RESULT(result);

        VE_CORE_TRACE("VkDevice created");

        // Initialize graphics and present queues
        if (queueIndices.GraphicsFamily.has_value())
        {
            vkGetDeviceQueue(m_Device, queueIndices.GraphicsFamily.value(), 0, &m_GraphicsQueue);
            VE_CORE_TRACE("Graphics queue initialized");
        }

        if (queueIndices.PresentFamily.has_value())
        {
            vkGetDeviceQueue(m_Device, queueIndices.PresentFamily.value(), 0, &m_PresentQueue);
            VE_CORE_TRACE("Present queue initialized");
        }
    }

    void VulkanCore::CreateSwapchain()
    {
        const auto &physicalDevice = m_PhysicalDevices.Selected();
        SwapchainSupportDetails swapchainSupport;
        swapchainSupport.Capabilities;
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice.Device, m_Surface, &swapchainSupport.Capabilities);

        uint32_t formatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice.Device, m_Surface, &formatCount, nullptr);
        if (formatCount != 0)
        {
            swapchainSupport.Formats.resize(formatCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice.Device, m_Surface, &formatCount, swapchainSupport.Formats.data());
        }

        uint32_t presentModeCount;
        vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice.Device, m_Surface, &presentModeCount, nullptr);
        if (presentModeCount != 0)
        {
            swapchainSupport.PresentModes.resize(presentModeCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice.Device, m_Surface, &presentModeCount, swapchainSupport.PresentModes.data());
        }

        // Choose swapchain parameters
        VkSurfaceFormatKHR surfaceFormat = ChooseSwapchainFormat(swapchainSupport.Formats);
        VkPresentModeKHR presentMode = ChooseSwapchainPresentMode(swapchainSupport.PresentModes);
        VkExtent2D extent = ChooseSwapchainExtent(swapchainSupport.Capabilities, m_Window);

        // Calculate image count
        uint32_t imageCount = swapchainSupport.Capabilities.minImageCount + 1;
        if (swapchainSupport.Capabilities.maxImageCount > 0 &&
            imageCount > swapchainSupport.Capabilities.maxImageCount)
        {
            imageCount = swapchainSupport.Capabilities.maxImageCount;
        }

        // Get queue indices
        PhysicalDeviceQueueFamilyIndices queueIndices = m_PhysicalDevices.GetQueueIndices(m_Surface);
        std::vector<uint32_t> queueIndicesArray;
        if (queueIndices.GraphicsFamily.value() != queueIndices.PresentFamily.value())
        {
            queueIndicesArray = {queueIndices.GraphicsFamily.value(), queueIndices.PresentFamily.value()};
        }

        // Create swapchain
        VkSwapchainCreateInfoKHR createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface = m_Surface;
        createInfo.minImageCount = imageCount;
        createInfo.imageFormat = surfaceFormat.format;
        createInfo.imageColorSpace = surfaceFormat.colorSpace;
        createInfo.imageExtent = extent;
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT; // For post-processing
        createInfo.preTransform = swapchainSupport.Capabilities.currentTransform;
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        createInfo.presentMode = presentMode;
        createInfo.clipped = VK_TRUE;
        createInfo.oldSwapchain = VK_NULL_HANDLE;

        if (!queueIndicesArray.empty())
        {
            createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            createInfo.queueFamilyIndexCount = static_cast<uint32_t>(queueIndicesArray.size());
            createInfo.pQueueFamilyIndices = queueIndicesArray.data();
        }
        else
        {
            createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
            createInfo.queueFamilyIndexCount = 1;
            createInfo.pQueueFamilyIndices = &queueIndices.GraphicsFamily.value();
        }

        VkResult result = vkCreateSwapchainKHR(m_Device, &createInfo, nullptr, &m_Swapchain);
        CHECK_VK_RESULT(result);

        m_SwapchainImageFormat = surfaceFormat.format;
        m_SwapchainExtent = extent;

        uint32_t swapchainImageCount;
        vkGetSwapchainImagesKHR(m_Device, m_Swapchain, &swapchainImageCount, nullptr);
        m_SwapchainImages.resize(swapchainImageCount);
        vkGetSwapchainImagesKHR(m_Device, m_Swapchain, &swapchainImageCount, m_SwapchainImages.data());

        VE_CORE_TRACE("Created VkSwapchainKHR with {0} images", m_SwapchainImages.size());
        VE_CORE_TRACE("\tFormat: {0}", static_cast<uint32_t>(m_SwapchainImageFormat));
        VE_CORE_TRACE("\tExtent: {0}x{1}", m_SwapchainExtent.width, m_SwapchainExtent.height);
        VE_CORE_TRACE("\tPresent Mode: {0}", static_cast<uint32_t>(presentMode));
    }

    void VulkanCore::CreateImageViews()
    {
        uint32_t layerCount = 1;
        uint32_t mipLevels = 1;
        m_SwapchainImageViews.resize(m_SwapchainImages.size());

        VkImageViewType viewFormat = VK_IMAGE_VIEW_TYPE_2D;
        VkImageAspectFlags aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

        for (uint32_t i = 0; i < m_SwapchainImages.size(); i++)
        {
            VkImageViewCreateInfo createInfo{};
            createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            createInfo.image = m_SwapchainImages[i];
            createInfo.viewType = viewFormat;
            createInfo.format = m_SwapchainImageFormat;
            createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.subresourceRange.aspectMask = aspectMask;
            createInfo.subresourceRange.baseMipLevel = 0;
            createInfo.subresourceRange.levelCount = mipLevels;
            createInfo.subresourceRange.baseArrayLayer = 0;
            createInfo.subresourceRange.layerCount = layerCount;

            VkResult result = vkCreateImageView(m_Device, &createInfo, nullptr, &m_SwapchainImageViews[i]);
            CHECK_VK_RESULT(result);
        }

        VE_CORE_TRACE("Created {0} swapchain VkImageView", m_SwapchainImageViews.size());
    }

    void VulkanCore::CreateCommandPool()
    {
        VkCommandPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.queueFamilyIndex = m_PhysicalDevices.GetQueueIndices(m_Surface).GraphicsFamily.value();

        VkResult result = vkCreateCommandPool(m_Device, &poolInfo, nullptr, &m_CommandPool);
        CHECK_VK_RESULT(result);

        VE_CORE_TRACE("VkCommandPool created");
    }

    VkSurfaceFormatKHR VulkanCore::ChooseSwapchainFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats) const
    {
        // Prefer SRGB format with 32-bit RGBA
        for (const auto &format : availableFormats)
        {
            if (format.format == VK_FORMAT_B8G8R8A8_SRGB &&
                format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
            {
                VE_CORE_TRACE("Selected swapchain format: VK_FORMAT_B8G8R8A8_SRGB");
                VE_CORE_TRACE("Selected swapchain color space: VK_COLOR_SPACE_SRGB_NONLINEAR_KHR");
                return format;
            }
        }

        VE_CORE_WARN("Preferred swapchain format not found, using first available");
        VE_CORE_WARN("Preferred swapchain color space not found, using first available");
        return availableFormats[0];
    }

    VkPresentModeKHR VulkanCore::ChooseSwapchainPresentMode(const std::vector<VkPresentModeKHR> &availableModes) const
    {
        // Prefer VK_PRESENT_MODE_MAILBOX_KHR format
        for (const auto &mode : availableModes)
        {
            if (mode == VK_PRESENT_MODE_MAILBOX_KHR)
            {
                VE_CORE_TRACE("Selected present mode: VK_PRESENT_MODE_MAILBOX_KHR");
                return mode;
            }
        }

        VE_CORE_TRACE("Mailbox present mode not available, using VK_PRESENT_MODE_FIFO_KHR");
        return VK_PRESENT_MODE_FIFO_KHR;
    }

    VkExtent2D VulkanCore::ChooseSwapchainExtent(const VkSurfaceCapabilitiesKHR &capabilities, GLFWwindow *window) const
    {
        if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
        {
            return capabilities.currentExtent;
        }

        int width, height;
        glfwGetFramebufferSize(window, &width, &height);

        VkExtent2D extent = {
            static_cast<uint32_t>(width),
            static_cast<uint32_t>(height)};

        extent.width = std::max(capabilities.minImageExtent.width,
                                std::min(capabilities.maxImageExtent.width, extent.width));
        extent.height = std::max(capabilities.minImageExtent.height,
                                 std::min(capabilities.maxImageExtent.height, extent.height));

        VE_CORE_TRACE("Calculated swapchain extent: {0}x{1}", extent.width, extent.height);
        return extent;
    }

} // namespace ve
