#pragma once

#include <vulkan/vulkan.h>

namespace VE
{
    class Instance;

    class DebugUtilsMessenger final
    {
    public:
        DebugUtilsMessenger(const Instance &instnace);
        ~DebugUtilsMessenger();

    public:
        VkDebugUtilsMessengerEXT m_Messenger = VK_NULL_HANDLE;
        const Instance &m_Instance;
    };

}