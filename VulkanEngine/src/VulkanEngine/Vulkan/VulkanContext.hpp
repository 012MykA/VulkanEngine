#pragma once

#include "VulkanEngine/Core/Base.hpp"
#include "VulkanConfig.hpp"
#include "VulkanPhysicalDevice.hpp"

#include <vulkan/vulkan.h>

struct GLFWwindow;

namespace ve
{
    class VulkanContext
    {
    public:
        VulkanContext(const VulkanConfig &config, GLFWwindow *window);
        ~VulkanContext();

        void DeviceWaitIdle() const;

    public:
        // Getters
        const VulkanPhysicalDevice &GetPhysicalDevice() const { return *m_PhysicalDevice; }
        VkDevice GetDevice() const { return m_Device; }
        VkQueue GetGraphicsQueue() const { return m_GraphicsQueue; }
        VkQueue GetPresentQueue() const { return m_PresentQueue; }

        VkSurfaceKHR GetSurface() const { return m_Surface; }

    private:
        void CreateInstance(const VulkanConfig &config);
        void CreateDebugCallback(const VulkanConfig &config);
        void CreateSurface(GLFWwindow *window);
        void CreateDevice(const PhysicalDeviceRequirements &requirements);

    private:
        VkInstance m_Instance = VK_NULL_HANDLE;
        VkDebugUtilsMessengerEXT m_DebugMessenger = VK_NULL_HANDLE;
        VkSurfaceKHR m_Surface = VK_NULL_HANDLE;

        Scope<VulkanPhysicalDevice> m_PhysicalDevice;

        VkDevice m_Device = VK_NULL_HANDLE;
        VkQueue m_GraphicsQueue = VK_NULL_HANDLE;
        VkQueue m_PresentQueue = VK_NULL_HANDLE;
    };

} // namespace ve
