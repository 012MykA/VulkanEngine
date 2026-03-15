#include "VulkanSwapchain.hpp"
#include "Debug/VulkanValidation.hpp"
#include "VulkanEngine/Core/Log.hpp"

#include <limits>

namespace ve
{
    VulkanSwapchain::VulkanSwapchain(
        VkDevice device, VkSurfaceKHR surface, const VulkanPhysicalDevice &physicalDevice,
        VkExtent2D windowExtent, VkSwapchainKHR oldSwapchain) : m_Device(device)
    {
        SwapchainSupportDetails details = physicalDevice.GetSwapchainSupport(surface);

        VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(details.Formats);
        VkPresentModeKHR presentMode = ChooseSwapPresentMode(details.PresentModes);
        VkExtent2D extent = ChooseSwapExtent(details.Capabilities, windowExtent);

        uint32_t imageCount = details.Capabilities.minImageCount + 1;
        if (details.Capabilities.maxImageCount > 0 && imageCount > details.Capabilities.maxImageCount)
        {
            imageCount = details.Capabilities.maxImageCount;
        }

        VkSwapchainCreateInfoKHR createInfo{
            .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
            .surface = surface,
            .minImageCount = imageCount,
            .imageFormat = surfaceFormat.format,
            .imageColorSpace = surfaceFormat.colorSpace,
            .imageExtent = extent,
            .imageArrayLayers = 1,
            .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
            .preTransform = details.Capabilities.currentTransform,
            .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
            .presentMode = presentMode,
            .clipped = VK_TRUE,
            .oldSwapchain = oldSwapchain,
        };

        auto indices = physicalDevice.GetQueueIndices();
        uint32_t queueFamilyIndices[] = {indices.GraphicsFamily.value(), indices.PresentFamily.value()};

        if (indices.GraphicsFamily != indices.PresentFamily)
        {
            createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            createInfo.queueFamilyIndexCount = 2;
            createInfo.pQueueFamilyIndices = queueFamilyIndices;
        }
        else
        {
            createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        }

        VkResult result = vkCreateSwapchainKHR(m_Device, &createInfo, nullptr, &m_Swapchain);
        CHECK_VK_RESULT(result);

        vkGetSwapchainImagesKHR(m_Device, m_Swapchain, &imageCount, nullptr);
        m_Images.resize(imageCount);
        vkGetSwapchainImagesKHR(m_Device, m_Swapchain, &imageCount, m_Images.data());

        m_ImageFormat = surfaceFormat.format;
        m_Extent = extent;

        VE_CORE_TRACE("VkSwapchainKHR created");
        VE_CORE_TRACE("\tImage count: {0}", m_Images.size());
        VE_CORE_TRACE("\tExtent: {0}x{1}", m_Extent.width, m_Extent.height);

        m_ImageViews.resize(m_Images.size());
        for (size_t i = 0; i < m_Images.size(); i++)
        {
            VkImageViewCreateInfo viewInfo{
                .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
                .image = m_Images[i],
                .viewType = VK_IMAGE_VIEW_TYPE_2D,
                .format = m_ImageFormat,
                .subresourceRange{
                    .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                    .baseMipLevel = 0,
                    .levelCount = 1,
                    .baseArrayLayer = 0,
                    .layerCount = 1,
                },
            };

            result = vkCreateImageView(m_Device, &viewInfo, nullptr, &m_ImageViews[i]);
            CHECK_VK_RESULT(result);
        }
        VE_CORE_TRACE("Created {0} swapchain image views", m_ImageViews.size());
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
                VE_CORE_TRACE("Selected swapchain present mode: VK_PRESENT_MODE_MAILBOX_KHR");
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

} // namespace ve
