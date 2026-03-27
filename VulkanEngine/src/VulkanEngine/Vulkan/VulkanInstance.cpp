#include "VulkanInstance.hpp"
#include "Debug/VulkanValidation.hpp"
#include "VulkanEngine/Core/Log.hpp"

namespace ve
{
    namespace
    {
        VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
            VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
            [[maybe_unused]] VkDebugUtilsMessageTypeFlagsEXT messageTypes,
            const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
            [[maybe_unused]] void *pUserData)
        {
            if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT)
                VE_CORE_TRACE("VulkanValidation:\n\t{0}", pCallbackData->pMessage);
            else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)
                VE_CORE_INFO("VulkanValidation:\n\t{0}", pCallbackData->pMessage);
            else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
                VE_CORE_WARN("VulkanValidation:\n\t{0}", pCallbackData->pMessage);
            else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
                VE_CORE_ERROR("VulkanValidation:\n\t{0}", pCallbackData->pMessage);
            else
                VE_CORE_ERROR("VulkanValidation (Unknown Severity):\n\t{0}", pCallbackData->pMessage);

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

    VulkanInstance::VulkanInstance(const InstanceDesc &desc)
    {
        CreateInstance(desc);

        if (desc.enableValidation && desc.debugMessenger.enableDebugMessenger)
            CreateDebugMessenger(desc.debugMessenger);
    }

    VulkanInstance::~VulkanInstance()
    {
        if (m_DebugMessenger != VK_NULL_HANDLE)
            DestroyDebugUtilsMessengerEXT(m_Instance, m_DebugMessenger, nullptr);

        if (m_Instance != VK_NULL_HANDLE)
            vkDestroyInstance(m_Instance, nullptr);
    }

    void VulkanInstance::CreateInstance(const InstanceDesc &desc)
    {
        if (desc.enableValidation && !CheckValidationLayerSupport(desc.validationLayers))
            throw std::runtime_error("[Vulkan] Required validation layers are not available");

        // --- AppInfo ----------------------------------------
        VkApplicationInfo appInfo{
            .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
            .pNext = nullptr,
            .pApplicationName = desc.appName,
            .applicationVersion = desc.appVersion,
            .pEngineName = desc.engineName,
            .engineVersion = desc.engineVersion,
            .apiVersion = desc.apiVersion,
        };

        // --- Extensions ----------------------------------------
        auto extensions = BuildExtensionList(desc);

        VkInstanceCreateInfo createInfo{
            .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
            .pApplicationInfo = &appInfo,
            .enabledExtensionCount = static_cast<uint32_t>(extensions.size()),
            .ppEnabledExtensionNames = extensions.data(),
        };

        VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};

        if (desc.enableValidation)
        {
            createInfo.enabledLayerCount = static_cast<uint32_t>(desc.validationLayers.size()),
            createInfo.ppEnabledLayerNames = desc.validationLayers.data();

            if (desc.debugMessenger.enableDebugMessenger)
            {
                debugCreateInfo = BuildMessengerCreateInfo(desc.debugMessenger);
                createInfo.pNext = &debugCreateInfo;
            }
        }

        VkResult result = vkCreateInstance(&createInfo, nullptr, &m_Instance);
        CHECK_VK_RESULT(result);

        // Logging
        VE_CORE_INFO("VulkanInstance created");
        VE_CORE_INFO("  API version: {}.{}.{}",
                     VK_API_VERSION_MAJOR(desc.apiVersion),
                     VK_API_VERSION_MINOR(desc.apiVersion),
                     VK_API_VERSION_PATCH(desc.apiVersion));

        VE_CORE_INFO("  Validation: {}", desc.enableValidation ? "ON" : "OFF");
        if (desc.enableValidation)
        {
            for (const char *layer : desc.validationLayers)
                VE_CORE_INFO("    layer: {}", layer);
        }

        VE_CORE_INFO("  Extensions ({}):", extensions.size());
        for (const char *ext : extensions)
            VE_CORE_INFO("     {}", ext);
    }

    void VulkanInstance::CreateDebugMessenger(const DebugMessengerDesc &desc)
    {
        VkDebugUtilsMessengerCreateInfoEXT createInfo = BuildMessengerCreateInfo(desc);

        VkResult result = CreateDebugUtilsMessengerEXT(m_Instance, &createInfo, nullptr, &m_DebugMessenger);
        CHECK_VK_RESULT(result);
    }

    bool VulkanInstance::CheckValidationLayerSupport(const std::vector<const char *> &layers)
    {
        uint32_t count = 0;
        vkEnumerateInstanceLayerProperties(&count, nullptr);
        std::vector<VkLayerProperties> available(count);
        vkEnumerateInstanceLayerProperties(&count, available.data());

        for (const char *name : layers)
        {
            bool found = false;
            for (const auto &props : available)
            {
                if (strcmp(name, props.layerName) == 0)
                {
                    found = true;
                    break;
                }
            }

            if (!found)
                return false;
        }
        return true;
    }

    std::vector<const char *> VulkanInstance::BuildExtensionList(const InstanceDesc &desc)
    {
        std::vector<const char *> extensions = desc.requiredExtensions;

        if (desc.enableValidation && desc.debugMessenger.enableDebugMessenger)
        {
            // Добавляем только если ещё нет — пользователь мог передать сам
            bool alreadyPresent = false;
            for (const char *ext : extensions)
                if (strcmp(ext, VK_EXT_DEBUG_UTILS_EXTENSION_NAME) == 0)
                {
                    alreadyPresent = true;
                    break;
                }

            if (!alreadyPresent)
                extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }

        uint32_t count = 0;
        vkEnumerateInstanceExtensionProperties(nullptr, &count, nullptr);
        std::vector<VkExtensionProperties> available(count);
        vkEnumerateInstanceExtensionProperties(nullptr, &count, available.data());

        for (const char *requested : extensions)
        {
            bool found = false;
            for (const auto &props : available)
                if (strcmp(requested, props.extensionName) == 0)
                {
                    found = true;
                    break;
                }

            if (!found)
            {
                std::string msg = "Required instance extension not supported: ";
                throw std::runtime_error(msg + requested);
            }
        }

        return extensions;
    }

    VkDebugUtilsMessengerCreateInfoEXT VulkanInstance::BuildMessengerCreateInfo(
        const DebugMessengerDesc &desc)
    {
        VkDebugUtilsMessengerCreateInfoEXT info{};
        info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        info.messageSeverity = desc.messageSeverity;
        info.messageType = desc.messageType;
        info.pfnUserCallback = desc.pfnUserCallback ? desc.pfnUserCallback : DebugCallback;
        info.pUserData = desc.pUserData;
        return info;
    }

} // namespace ve
