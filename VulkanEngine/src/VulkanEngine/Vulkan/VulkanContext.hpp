#pragma once

#include "VulkanEngine/Core/Base.hpp"
#include "VulkanConfig.hpp"
#include "VulkanPhysicalDevice.hpp"
#include "VulkanSwapchain.hpp"

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

        bool IsFramebufferResized() const { return m_FramebufferResized; }
        void ResetFramebufferResized() { m_FramebufferResized = false; }
        void RecreateSwapchain();

        void OnWindowResize(uint32_t width, uint32_t height);

    public:
        // Getters
        const VulkanPhysicalDevice &GetPhysicalDevice() const { return *m_PhysicalDevice; }
        VkDevice GetDevice() const { return m_Device; }
        VkQueue GetGraphicsQueue() const { return m_GraphicsQueue; }
        VkQueue GetPresentQueue() const { return m_PresentQueue; }

        VkSwapchainKHR GetSwapchain() const { return m_Swapchain->GetSwapchain(); }
        VkExtent2D GetSwapchainExtent() const { return m_Swapchain->GetExtent(); }
        VkFormat GetSwapchainFormat() const { return m_Swapchain->GetImageFormat(); }
        uint32_t GetSwapchainImageCount() const { return m_Swapchain->GetImageCount(); }
        VkImageView GetSwapchainImageView(uint32_t index) const { return m_Swapchain->GetImageView(index); }

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

        bool m_FramebufferResized = false;
        VkExtent2D m_FramebufferExtent = {0, 0};
    };

} // namespace ve
