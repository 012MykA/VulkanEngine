#pragma once

#include <vulkan/vulkan.h>

#include <string>
#include <cstdint>
#include <vector>

namespace ve
{
    struct DebugCallbackConfig
    {
        bool EnableDebugMessenger = false;

        VkDebugUtilsMessageSeverityFlagsEXT MessageSeverity =
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;

        VkDebugUtilsMessageTypeFlagsEXT MessageType =
            VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    };

    struct VulkanConfig
    {
        std::string AppName;
        uint32_t AppVersion;
        std::string EngineName;
        uint32_t EngineVersion;
        uint32_t ApiVersion;
        std::vector<const char *> InstanceExtensions = {};
        bool EnableValidationLayers = false;
        std::vector<const char *> ValidationLayers = {};
        DebugCallbackConfig DebugConfig;
    };

} // namespace ve
