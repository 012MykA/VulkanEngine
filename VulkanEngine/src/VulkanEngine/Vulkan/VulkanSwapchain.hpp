#pragma once

#include <vulkan/vulkan.h>

#include <cstdint>
#include <vector>

namespace ve
{
    class VulkanPhysicalDevice;
    class VulkanLogicalDevice;
    class VulkanSurface;

    struct SwapchainDesc
    {
        uint32_t width = 0;
        uint32_t height = 0;

        std::vector<VkSurfaceFormatKHR> preferredFormats = {
            {VK_FORMAT_B8G8R8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR},
            {VK_FORMAT_R8G8B8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR},
        };

        std::vector<VkPresentModeKHR> preferredPresentModes = {
            VK_PRESENT_MODE_MAILBOX_KHR, // Triple buffering
            VK_PRESENT_MODE_FIFO_KHR,    // VSync
        };
    };

    class VulkanSwapchain
    {
    public:
        VulkanSwapchain(const VulkanPhysicalDevice &physicalDevice,
                        const VulkanLogicalDevice &logicalDevice,
                        const VulkanSurface &surface,
                        const SwapchainDesc &desc);
        ~VulkanSwapchain();

        VulkanSwapchain(const VulkanSwapchain &) = delete;
        VulkanSwapchain &operator=(const VulkanSwapchain &) = delete;

        void Recreate(uint32_t newWidth, uint32_t newHeight);

        // Getters
        VkSwapchainKHR GetVkHandle() const { return m_Swapchain; }
        VkFormat GetFormat() const { return m_Format; }
        VkExtent2D GetExtent() const { return m_Extent; }
        uint32_t GetImageCount() const { return static_cast<uint32_t>(m_Images.size()); }
        const std::vector<VkImage> &GetImages() const { return m_Images; }
        const std::vector<VkImageView> &GetImageViews() const { return m_ImageViews; }

    private:
        void Create(uint32_t width, uint32_t height, VkSwapchainKHR oldSwapchain = VK_NULL_HANDLE);

        void CreateImageViews();

        VkSurfaceFormatKHR ChooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &available) const;
        VkPresentModeKHR ChoosePresentMode(const std::vector<VkPresentModeKHR> &available) const;
        VkExtent2D ChooseExtent(const VkSurfaceCapabilitiesKHR &capabilities, uint32_t width, uint32_t height) const;

    private:
        const VulkanPhysicalDevice &m_PhysicalDevice;
        const VulkanLogicalDevice &m_LogicalDevice;
        VkSurfaceKHR m_Surface = VK_NULL_HANDLE;
        SwapchainDesc m_Desc;

        VkSwapchainKHR m_Swapchain = VK_NULL_HANDLE;
        VkFormat m_Format = VK_FORMAT_UNDEFINED;
        VkExtent2D m_Extent = {};

        std::vector<VkImage> m_Images;
        std::vector<VkImageView> m_ImageViews;
    };

} // namespace ve
