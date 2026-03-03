#pragma once

#include "VulkanDevice.hpp"

#include <vulkan/vulkan.h>

#include <string>

struct GLFWwindow;

namespace ve
{
    class VulkanCore
    {
    public:
        VulkanCore();
        ~VulkanCore();

        void Init(const std::string &appName, GLFWwindow* window);

    private:
        void CreateInstance(const std::string &appName);
        void CreateDebugCallback();
        void CreateSurface(GLFWwindow* window);

    private:
        VkInstance m_Instance = VK_NULL_HANDLE;
        VkDebugUtilsMessengerEXT m_DebugMessenger = VK_NULL_HANDLE;
        VkSurfaceKHR m_Surface = VK_NULL_HANDLE;
        VulkanPhysicalDevices m_PhysicalDevices;
        uint32_t m_QueueFamily = 0;
    };

} // namespace ve
