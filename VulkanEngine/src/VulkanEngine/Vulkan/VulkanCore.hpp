#pragma once

#include "VulkanEngine/Core/Base.hpp"
#include "VulkanConfig.hpp"
#include "VulkanPhysicalDevice.hpp"
#include "VulkanSwapchain.hpp"
#include "VulkanRenderPass.hpp"

#include <vulkan/vulkan.h>

struct GLFWwindow;

namespace ve
{
    class VulkanCore
    {
    public:
        VulkanCore();
        ~VulkanCore();

        void Init(const VulkanConfig &config, GLFWwindow *window);

    private:
        void CreateInstance(const VulkanConfig &config);
        void CreateDebugCallback(const VulkanConfig &config);
        void CreateSurface(GLFWwindow *window);
        void CreateDevice(const PhysicalDeviceRequirements &requirements);

    private:
        VkInstance m_Instance = VK_NULL_HANDLE;
        VkDebugUtilsMessengerEXT m_DebugMessenger = VK_NULL_HANDLE;
        VkSurfaceKHR m_Surface = VK_NULL_HANDLE;
        Scope<VulkanPhysicalDevice> m_PhysicalDevice = nullptr;
        VkDevice m_Device = VK_NULL_HANDLE;
        VkQueue m_GraphicsQueue = VK_NULL_HANDLE;
        VkQueue m_PresentQueue = VK_NULL_HANDLE;
        Scope<VulkanSwapchain> m_Swapchain = nullptr;
        Scope<VulkanRenderPass> m_RenderPass = nullptr;
    };

} // namespace ve
