#pragma once

#include "VulkanConfig.hpp"
#include "VulkanPhysicalDevices.hpp"

#include <vulkan/vulkan.h>
#include <vector>

struct GLFWwindow;

namespace ve
{
    class VulkanCore
    {
    public:
        VulkanCore();
        ~VulkanCore();

        void Init(const VulkanConfig &config, GLFWwindow *window);

        void CreateCommandBuffers(uint32_t count, VkCommandBuffer *commandBuffers);

    public:
        // Getters
        VkDevice GetDevice() const { return m_Device; }
        VkSwapchainKHR GetSwapchain() const { return m_Swapchain; }
        VkFormat GetSwapchainImageFormat() const { return m_SwapchainImageFormat; }
        VkExtent2D GetSwapchainExtent() const { return m_SwapchainExtent; }
        const std::vector<VkImage> &GetSwapchainImages() const { return m_SwapchainImages; }
        VkQueue GetGraphicsQueue() const { return m_GraphicsQueue; }
        VkQueue GetPresentQueue() const { return m_PresentQueue; }

    private:
        void CreateInstance(const VulkanConfig &config);
        void CreateDebugCallback(const VulkanConfig &config);
        void CreateSurface(GLFWwindow *window);
        void CreateDevice(const PhysicalDeviceRequirements &requirements);
        void CreateSwapchain();
        void CreateImageViews();
        void CreateCommandPool();

        // Helper methods for swapchain creation
        VkSurfaceFormatKHR ChooseSwapchainFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats) const;
        VkPresentModeKHR ChooseSwapchainPresentMode(const std::vector<VkPresentModeKHR> &availableModes) const;
        VkExtent2D ChooseSwapchainExtent(const VkSurfaceCapabilitiesKHR &capabilities, GLFWwindow *window) const;

    private:
        GLFWwindow *m_Window = nullptr;
        VkInstance m_Instance = VK_NULL_HANDLE;
        VkDebugUtilsMessengerEXT m_DebugMessenger = VK_NULL_HANDLE;
        VkSurfaceKHR m_Surface = VK_NULL_HANDLE;
        VulkanPhysicalDevices m_PhysicalDevices;
        VkDevice m_Device = VK_NULL_HANDLE;
        VkQueue m_GraphicsQueue = VK_NULL_HANDLE;
        VkQueue m_PresentQueue = VK_NULL_HANDLE;
        VkSwapchainKHR m_Swapchain = VK_NULL_HANDLE;
        std::vector<VkImage> m_SwapchainImages;
        VkFormat m_SwapchainImageFormat = VK_FORMAT_UNDEFINED;
        VkExtent2D m_SwapchainExtent = {0, 0};
        std::vector<VkImageView> m_SwapchainImageViews;
        VkCommandPool m_CommandPool = VK_NULL_HANDLE;
    };

} // namespace ve
