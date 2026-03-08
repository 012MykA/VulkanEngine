#pragma once

#include "VulkanPhysicalDevice.hpp"

#include <vulkan/vulkan.h>

#include <vector>

namespace ve
{
    class VulkanSwapchain
    {
    public:
        VulkanSwapchain(VkDevice device, VkSurfaceKHR surface, const VulkanPhysicalDevice &physicalDevice,
                        VkExtent2D windowExtent, VkSwapchainKHR oldSwapchain = VK_NULL_HANDLE);
        ~VulkanSwapchain();

    public:
        // Getters
        VkSwapchainKHR GetSwapchain() const { return m_Swapchain; }

        VkFormat GetImageFormat() const { return m_ImageFormat; }
        VkExtent2D GetExtent() const { return m_Extent; }

        VkImageView GetImageView(uint32_t index) const { return m_ImageViews[index]; }
        const std::vector<VkImageView> &GetImageViews() const { return m_ImageViews; }

    private:
        void CreateSwapchain(VkExtent2D windowExtent);
        void CreateImageViews();

    private:
        VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats);
        VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes);
        VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities, VkExtent2D extent);

    private:
        VkDevice m_Device;
        VkSwapchainKHR m_Swapchain = VK_NULL_HANDLE;

        VkFormat m_ImageFormat;
        VkExtent2D m_Extent;

        std::vector<VkImage> m_Images;
        std::vector<VkImageView> m_ImageViews;
    };

} // namespace ve
