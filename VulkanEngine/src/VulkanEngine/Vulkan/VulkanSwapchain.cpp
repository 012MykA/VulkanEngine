#include "VulkanSwapchain.hpp"
#include "Debug/Validation.hpp"
#include "VulkanEngine/Core/Log.hpp"

#include <limits>

namespace ve
{
    VulkanSwapchain::VulkanSwapchain(VkDevice device, VkSurfaceKHR surface, const VulkanPhysicalDevice &physicalDevice,
                        VkExtent2D windowExtent, VkSwapchainKHR oldSwapchain)
    {
        CreateSwapchain(windowExtent);
        CreateImageViews();
    }

    VulkanSwapchain::~VulkanSwapchain()
    {
        for (auto imageView : m_ImageViews)
            vkDestroyImageView(m_Device, imageView, nullptr);
        VE_CORE_TRACE("Destroyed {0} swapchain image views", m_ImageViews.size());

        vkDestroySwapchainKHR(m_Device, m_Swapchain, nullptr);
        VE_CORE_TRACE("VkSwapchainKHR destroyed");
    }

    VkSurfaceFormatKHR VulkanSwapchain::ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats)
    {
        for (const auto &availableFormat : availableFormats)
        {
            if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
            {
                VE_CORE_TRACE("Selected preferred swapchain format:");
                VE_CORE_TRACE("\tFormat: VK_FORMAT_B8G8R8A8_SRGB");
                VE_CORE_TRACE("\tColor space: VK_COLOR_SPACE_SRGB_NONLINEAR_KHR");
                return availableFormat;
            }
        }

        VE_CORE_TRACE("Preferred swapchain format not available, falling back to first available format");
        return availableFormats[0];
    }

    VkPresentModeKHR VulkanSwapchain::ChooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes)
    {
        for (const auto &availablePresentMode : availablePresentModes)
        {
            if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
            {
                VE_CORE_TRACE("Selected swapchain present mode: VK_FORMAT_B8G8R8A8_SRGB");
                return availablePresentMode;
            }
        }

        VE_CORE_TRACE("Preferred swapchain present mode not available, selected VK_PRESENT_MODE_FIFO_KHR");
        return VK_PRESENT_MODE_FIFO_KHR;
    }

    VkExtent2D VulkanSwapchain::ChooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities, VkExtent2D extent)
    {
        if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
        {
            return capabilities.currentExtent;
        }
        else
        {
            VkExtent2D actualExtent = extent;

            actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
            actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

            return actualExtent;
        }
    }

    void VulkanSwapchain::CreateImageViews()
    {
        m_ImageViews.resize(m_Images.size());
        for (size_t i = 0; i < m_Images.size(); i++)
        {
        }
    }

} // namespace ve
