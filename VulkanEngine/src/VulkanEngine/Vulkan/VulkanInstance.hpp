#pragma once

#include <vulkan/vulkan.h>

#include <cstdint>
#include <vector>

namespace ve
{
    struct DebugMessengerDesc
    {
        bool enableDebugMessenger = false;

        VkDebugUtilsMessageSeverityFlagsEXT messageSeverity =
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;

        VkDebugUtilsMessageTypeFlagsEXT messageType =
            VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;

        PFN_vkDebugUtilsMessengerCallbackEXT pfnUserCallback = nullptr;
        void *pUserData = nullptr;
    };

    struct InstanceDesc
    {
        const char *appName = "Vulkan App";
        uint32_t appVersion = VK_MAKE_VERSION(1, 0, 0);
        const char *engineName = "VulkanEngine";
        uint32_t engineVersion = VK_MAKE_VERSION(1, 0, 0);
        uint32_t apiVersion = VK_API_VERSION_1_3;

        std::vector<const char *> requiredExtensions;

        bool enableValidation = false;
        std::vector<const char *> validationLayers = {
            "VK_LAYER_KHRONOS_validation",
        };

        DebugMessengerDesc debugMessenger;
    };

    class VulkanInstance
    {
    public:
        explicit VulkanInstance(const InstanceDesc &desc);
        ~VulkanInstance();

        VulkanInstance(const VulkanInstance &) = delete;
        VulkanInstance &operator=(const VulkanInstance &) = delete;

        VkInstance GetVkHandle() const { return m_Instance; }

    private:
        void CreateInstance(const InstanceDesc &desc);
        void CreateDebugMessenger(const DebugMessengerDesc &desc);

    private:
        bool CheckValidationLayerSupport(const std::vector<const char *> &layers);
        std::vector<const char *> BuildExtensionList(const InstanceDesc &desc);
        VkDebugUtilsMessengerCreateInfoEXT BuildMessengerCreateInfo(const DebugMessengerDesc &desc);

    private:
        VkInstance m_Instance = VK_NULL_HANDLE;
        VkDebugUtilsMessengerEXT m_DebugMessenger = VK_NULL_HANDLE;
        bool m_ValidationEnabled = false;
    };

} // namespace ve
