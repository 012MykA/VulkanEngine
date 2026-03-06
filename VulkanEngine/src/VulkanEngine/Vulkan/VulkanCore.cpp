#include "VulkanCore.hpp"
#include "VulkanEngine/Core/Base.hpp"
#include "VulkanEngine/Core/Log.hpp"
#include "Debug/Validation.hpp"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

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
                VE_CORE_TRACE("Vulkan Validation: {0}", pCallbackData->pMessage);
            }
            else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)
            {
                VE_CORE_INFO("Vulkan Validation: {0}", pCallbackData->pMessage);
            }
            else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
            {
                VE_CORE_WARN("Vulkan Validation: {0}", pCallbackData->pMessage);
            }
            else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
            {
                VE_CORE_ERROR("Vulkan Validation: {0}", pCallbackData->pMessage);
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
        VE_CORE_TRACE("---------------------------------------");

        vkDestroySurfaceKHR(m_Instance, m_Surface, nullptr);
        VE_CORE_TRACE("VkSurfaceKHR destroyed");

        DestroyDebugUtilsMessengerEXT(m_Instance, m_DebugMessenger, nullptr);
        VE_CORE_TRACE("VkDebugUtilsMessengerEXT destroyed");

        vkDestroyInstance(m_Instance, nullptr);
        VE_CORE_TRACE("VkInstance destroyed");
    }

    void VulkanCore::Init(const VulkanConfig &config, GLFWwindow *window)
    {
        VE_CORE_INFO("Initializing VulkanCore...");
        CreateInstance(config);
        CreateDebugCallback(config);
        CreateSurface(window);
        m_PhysicalDevices.Init(m_Instance, m_Surface);
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

} // namespace ve
