#include "VulkanCore.hpp"
#include "VulkanEngine/Core/Base.hpp"
#include "VulkanEngine/Core/Log.hpp"
#include "Validation.hpp"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace ve
{
    namespace
    {
        const char *GetDebugSeverityStr(VkDebugUtilsMessageSeverityFlagBitsEXT severity)
        {
            switch (severity)
            {
            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
                return "Verbose";

            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
                return "Info";

            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
                return "Warning";

            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
                return "Error";

            default:
                throw std::invalid_argument("Invalid severity code");
            }
        }

        const char *GetDebugTypeStr(VkDebugUtilsMessageTypeFlagsEXT type)
        {
            switch (type)
            {
            case VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT:
                return "General";

            case VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT:
                return "Validation";

            case VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT:
                return "Performance";

            case VK_DEBUG_UTILS_MESSAGE_TYPE_DEVICE_ADDRESS_BINDING_BIT_EXT:
                return "Device address binding";

            default:
                throw std::invalid_argument("Invalid type code");
            }
        }

        const char *GetObjectTypeStr(VkObjectType type)
        {
            switch (type)
            {
            case VK_OBJECT_TYPE_INSTANCE:
                return "Instance";

            case VK_OBJECT_TYPE_DEVICE:
                return "Device";

            case VK_OBJECT_TYPE_BUFFER:
                return "Buffer";

            case VK_OBJECT_TYPE_IMAGE:
                return "Image";

            case VK_OBJECT_TYPE_COMMAND_BUFFER:
                return "CommandBuffer";

            case VK_OBJECT_TYPE_QUEUE:
                return "Queue";

            case VK_OBJECT_TYPE_RENDER_PASS:
                return "RenderPass";

            case VK_OBJECT_TYPE_PIPELINE:
                return "Pipeline";

            case VK_OBJECT_TYPE_DESCRIPTOR_SET:
                return "DescriptorSet";

            default:
                return "UnknownObject";
            }
        }

        VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
            VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
            VkDebugUtilsMessageTypeFlagsEXT messageType,
            const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
            void *pUserData)
        {
            VE_CORE_ERROR("========== Vulkan Validation ==========");
            VE_CORE_ERROR("Message: {0}", pCallbackData->pMessage);
            VE_CORE_ERROR("Severity: {0}", GetDebugSeverityStr(messageSeverity));
            VE_CORE_ERROR("Type: {0}", GetDebugTypeStr(messageType));

            if (pCallbackData->objectCount > 0)
            {
                VE_CORE_ERROR("Objects ({0}):", pCallbackData->objectCount);

                for (uint32_t i = 0; i < pCallbackData->objectCount; i++)
                {
                    const auto &obj = pCallbackData->pObjects[i];

                    VE_CORE_ERROR(
                        "  [{0}] Type: {1}, Handle: 0x{2}, Name: {3}",
                        i,
                        GetObjectTypeStr(obj.objectType),
                        (uint64_t)obj.objectHandle,
                        obj.pObjectName ? obj.pObjectName : "Unnamed");
                }
            }

            VE_CORE_ERROR("=======================================");

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

        vkDestroySurfaceKHR(m_Instance, m_Surface, nullptr);
        VE_CORE_TRACE("VkSurfaceKHR destroyed");
        if constexpr (validation::enabled)
        {
            DestroyDebugUtilsMessengerEXT(m_Instance, m_DebugMessenger, nullptr);
            VE_CORE_TRACE("VkDebugUtilsMessengerEXT destroyed");
        }
        vkDestroyInstance(m_Instance, nullptr);
        VE_CORE_TRACE("VkInstance destroyed");
    }

    void VulkanCore::Init(const std::string &appName, GLFWwindow* window)
    {
        CreateInstance(appName);
        if constexpr (validation::enabled)
        {
            CreateDebugCallback();
        }
        CreateSurface(window);
        m_PhysicalDevices.Init(m_Instance, m_Surface);
        m_QueueFamily = m_PhysicalDevices.SelectDevice(VK_QUEUE_GRAPHICS_BIT, true);
    }

    void VulkanCore::CreateInstance(const std::string &appName)
    {
        const std::vector<const char *> validationLayers =
            validation::enabled ? std::vector<const char *>{"VK_LAYER_KHRONOS_validation"}
                                : std::vector<const char *>{};

        if constexpr (validation::enabled)
        {
            validation::CheckValidationLayerSupport(validationLayers);
        }

        std::vector<const char *> extensions = {
            VK_KHR_SURFACE_EXTENSION_NAME,
#if defined(VE_PLATFORM_WINDOWS)
            "VK_KHR_win32_surface",
#elif defined(VE_PLATFORM_LINUX)
            "VK_KHR_wayland_surface",
#elif defined(VE_PLATFORM_APPLE)
            "VK_MVK_macos_surface",
#endif
        };
        if constexpr (validation::enabled)
        {
            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }

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

        validation::CheckVk(vkCreateInstance(&createInfo, nullptr, &m_Instance), "create instance");
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

        validation::CheckVk(CreateDebugUtilsMessengerEXT(m_Instance, &createInfo, nullptr, &m_DebugMessenger),
                            "setup Vulkan debug callback");
        VE_CORE_TRACE("VkDebugUtilsMessengerEXT created");
    }

    void VulkanCore::CreateSurface(GLFWwindow *window)
    {
        validation::CheckVk(glfwCreateWindowSurface(m_Instance, window, nullptr, &m_Surface),
                            "create window surface");
        VE_CORE_TRACE("VkSurfaceKHR created");
    }

} // namespace ve
