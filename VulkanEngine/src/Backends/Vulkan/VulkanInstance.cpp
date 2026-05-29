#include "VulkanInstance.hpp"
#include "Debug/VulkanValidation.hpp"
#include "VulkanEngine/Core/Log.hpp"

#include <cstring>
#include <stdexcept>
#include <algorithm>

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
                VE_CORE_TRACE("VulkanValidation:\n\t{}", pCallbackData->pMessage);
            else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)
                VE_CORE_INFO("VulkanValidation:\n\t{}", pCallbackData->pMessage);
            else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
                VE_CORE_WARN("VulkanValidation:\n\t{}", pCallbackData->pMessage);
            else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
                VE_CORE_ERROR("VulkanValidation:\n\t{}", pCallbackData->pMessage);
            else
                VE_CORE_ERROR("VulkanValidation (Unknown Severity):\n\t{}", pCallbackData->pMessage);

            return VK_FALSE;
        }
    }

    VulkanInstance::VulkanInstance(const InstanceDesc &desc)
    {
        CreateInstance(desc);
    }

    VulkanInstance::~VulkanInstance()
    {
        if (m_Instance != VK_NULL_HANDLE)
            vkDestroyInstance(m_Instance, nullptr);
    }

    void VulkanInstance::CreateInstance(const InstanceDesc &desc)
    {
        if (desc.enableValidation && !CheckValidationLayerSupport(desc.validationLayers))
            throw std::runtime_error("Required validation layers are not available");

        VkApplicationInfo appInfo{
            .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
            .pNext = nullptr,
            .pApplicationName = desc.appName,
            .applicationVersion = desc.appVersion,
            .pEngineName = desc.engineName,
            .engineVersion = desc.engineVersion,
            .apiVersion = desc.apiVersion,
        };

        auto extensions = BuildExtensionList(desc);

        VkInstanceCreateInfo createInfo{
            .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
            .pNext = nullptr,
            .pApplicationInfo = &appInfo,
            .enabledLayerCount = 0,
            .ppEnabledLayerNames = nullptr,
            .enabledExtensionCount = static_cast<uint32_t>(extensions.size()),
            .ppEnabledExtensionNames = extensions.data(),
        };

        VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};

        if (desc.enableValidation)
        {
            createInfo.enabledLayerCount = static_cast<uint32_t>(desc.validationLayers.size());
            createInfo.ppEnabledLayerNames = desc.validationLayers.data();

            if (desc.debugMessenger.enableDebugMessenger)
            {
                debugCreateInfo = {
                    .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
                    .pNext = nullptr,
                    .messageSeverity = desc.debugMessenger.messageSeverity,
                    .messageType = desc.debugMessenger.messageType,
                    .pfnUserCallback = desc.debugMessenger.pfnUserCallback
                                           ? desc.debugMessenger.pfnUserCallback
                                           : DebugCallback,
                    .pUserData = desc.debugMessenger.pUserData,
                };

                createInfo.pNext = &debugCreateInfo;
            }
        }

        VkResult result = vkCreateInstance(&createInfo, nullptr, &m_Instance);
        CHECK_VK_RESULT(result);

#if !defined(DISABLE_VULKAN_LOGGING)
        VE_CORE_TRACE("_________________________________________________");
        VE_CORE_TRACE("Vulkan Version: {}.{}.{}",
                      VK_VERSION_MAJOR(desc.apiVersion),
                      VK_VERSION_MINOR(desc.apiVersion),
                      VK_VERSION_PATCH(desc.apiVersion));

        VE_CORE_TRACE("_________________________________________________");
        VE_CORE_TRACE("Available Instance Layers:");
        std::vector<VkLayerProperties> availableLayers = GetAvailableLayers();
        for (auto &layer : availableLayers)
        {
            VE_CORE_TRACE("{} (v. {}.{}.{} {}) : {}",
                          layer.layerName,
                          VK_VERSION_MAJOR(layer.specVersion),
                          VK_VERSION_MINOR(layer.specVersion),
                          VK_VERSION_PATCH(layer.specVersion),
                          layer.implementationVersion,
                          layer.description);
        }

        VE_CORE_TRACE("_________________________________________________");
        VE_CORE_TRACE("Available Instance Extensions:");
        std::vector<VkExtensionProperties> availableExtensions = GetAvailableExtensions();
        for (auto &ext : availableExtensions)
        {
            // clang-format off
            auto it = std::find_if(extensions.begin(), extensions.end(),
            [&ext](const char* reqExt)
            {
                return std::strcmp(reqExt, ext.extensionName) == 0;
            });
            // clang-format on
            bool isUsed = (it != extensions.end());

            VE_CORE_TRACE("[{}] {} (v. {}.{}.{})",
                          isUsed ? 'x' : ' ',
                          ext.extensionName,
                          VK_VERSION_MAJOR(ext.specVersion),
                          VK_VERSION_MINOR(ext.specVersion),
                          VK_VERSION_PATCH(ext.specVersion));
        }
#endif
    }

    std::vector<VkLayerProperties> VulkanInstance::GetAvailableLayers()
    {
        uint32_t count = 0;
        CHECK_VK_RESULT(vkEnumerateInstanceLayerProperties(&count, nullptr));

        std::vector<VkLayerProperties> available(count);
        CHECK_VK_RESULT(vkEnumerateInstanceLayerProperties(&count, available.data()));

        return std::move(available);
    }

    std::vector<VkExtensionProperties> VulkanInstance::GetAvailableExtensions()
    {
        uint32_t count = 0;
        CHECK_VK_RESULT(vkEnumerateInstanceExtensionProperties(nullptr, &count, nullptr));

        std::vector<VkExtensionProperties> available(count);
        CHECK_VK_RESULT(vkEnumerateInstanceExtensionProperties(nullptr, &count, available.data()));

        return std::move(available);
    }

    bool VulkanInstance::CheckValidationLayerSupport(const std::vector<const char *> &layers)
    {
        std::vector<VkLayerProperties> available = GetAvailableLayers();

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
            bool alreadyPresent = false;

            for (const char *ext : extensions)
            {
                if (strcmp(ext, VK_EXT_DEBUG_UTILS_EXTENSION_NAME) == 0)
                {
                    alreadyPresent = true;
                    break;
                }
            }

            if (!alreadyPresent)
                extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }

        std::vector<VkExtensionProperties> available = GetAvailableExtensions();

        for (const char *requested : extensions)
        {
            bool found = false;

            for (const auto &props : available)
            {
                if (strcmp(requested, props.extensionName) == 0)
                {
                    found = true;
                    break;
                }
            }

            if (!found)
            {
                throw std::runtime_error(std::string("Required instance extension not supported: ") + requested);
            }
        }

        return extensions;
    }

    VkDebugUtilsMessengerCreateInfoEXT VulkanInstance::BuildMessengerCreateInfo(
        const DebugMessengerDesc &desc)
    {
        VkDebugUtilsMessengerCreateInfoEXT info{};
        info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        info.pNext = nullptr;
        info.messageSeverity = desc.messageSeverity;
        info.messageType = desc.messageType;
        info.pfnUserCallback = desc.pfnUserCallback ? desc.pfnUserCallback : DebugCallback;
        info.pUserData = desc.pUserData;
        return info;
    }

} // namespace ve
