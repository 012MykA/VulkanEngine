#include "VulkanCore.hpp"
#include "VulkanEngine/Core/Base.hpp"
#include "VulkanEngine/Core/Log.hpp"

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

    void VulkanCore::Init(const std::string &appName, GLFWwindow *window)
    {
        CreateInstance(appName);
        CreateDebugCallback();
        CreateSurface(window);
    }

    void VulkanCore::CreateInstance(const std::string &appName)
    {
        const std::vector<const char *> validationLayers = std::vector<const char *>{"VK_LAYER_KHRONOS_validation"};

        // CheckValidationLayerSupport(validationLayers); // TODO

        std::vector<const char *> extensions = {
            VK_KHR_SURFACE_EXTENSION_NAME,
            VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
#if defined(VE_PLATFORM_WINDOWS)
            "VK_KHR_win32_surface",
#elif defined(VE_PLATFORM_LINUX)
            "VK_KHR_wayland_surface",
#elif defined(VE_PLATFORM_APPLE)
            "VK_MVK_macos_surface",
#endif
        };

        VE_CORE_TRACE("Instance extensions:");
        for (auto extension : extensions)
        {
            VE_CORE_TRACE("\t{0}", extension);
        }

        VkApplicationInfo appInfo{};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = appName.c_str();
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = "VulkanEngine";
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_3;

        VkInstanceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;
        createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
        createInfo.ppEnabledExtensionNames = extensions.data();
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();

        vkCreateInstance(&createInfo, nullptr, &m_Instance);
        VE_CORE_TRACE("VkInstance created");
    }

    void VulkanCore::CreateDebugCallback()
    {
        VkDebugUtilsMessengerCreateInfoEXT createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                                     VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
                                     VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                     VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                                 VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                                 VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        createInfo.pfnUserCallback = DebugCallback;
        createInfo.pUserData = nullptr; // Optional

        CreateDebugUtilsMessengerEXT(m_Instance, &createInfo, nullptr, &m_DebugMessenger);
        VE_CORE_TRACE("VkDebugUtilsMessengerEXT created");
    }

    void VulkanCore::CreateSurface(GLFWwindow *window)
    {
        glfwCreateWindowSurface(m_Instance, window, nullptr, &m_Surface);
        VE_CORE_TRACE("VkSurfaceKHR created");
    }

} // namespace ve
