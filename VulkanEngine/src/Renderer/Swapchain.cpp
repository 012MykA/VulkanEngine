#include "Swapchain.hpp"
#include "Device.hpp"
#include "Surface.hpp"
#include "Window.hpp"
#include "Validation.hpp"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <limits>

namespace VE
{
    Swapchain::Swapchain(const Device &device, const Surface &surface, const Window &window) : m_Device(device), m_Surface(surface)
    {
        CreateSwapchain(window);
        CreateImageViews();
    }

    Swapchain::~Swapchain()
    {
        for (auto imageView : m_ImageViews)
            vkDestroyImageView(m_Device.Handle(), imageView, nullptr);

        vkDestroySwapchainKHR(m_Device.Handle(), m_Swapchain, nullptr);
    }

    void Swapchain::CreateSwapchain(const Window &window)
    {
        auto support = m_Device.QuerySwapchainSupport(m_Device.GetPhysicalDevice());

        auto surfaceFormat = ChooseSurfaceFormat(support.Formats);
        auto presentMode = ChoosePresentMode(support.PresentModes);
        auto extent = ChooseExtent(support.Capabilities, window);

        uint32_t imageCount = support.Capabilities.minImageCount + 1;

        if (support.Capabilities.maxImageCount > 0 && imageCount > support.Capabilities.maxImageCount)
        {
            imageCount = support.Capabilities.maxImageCount;
        }

        VkSwapchainCreateInfoKHR createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface = m_Surface.Handle();

        createInfo.minImageCount = imageCount;
        createInfo.imageFormat = surfaceFormat.format;
        createInfo.imageColorSpace = surfaceFormat.colorSpace;
        createInfo.imageExtent = extent;
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        const auto &indices = m_Device.Queues();

        uint32_t queueFamilyIndices[] = {
            indices.GraphicsFamily.value(),
            indices.PresentFamily.value(),
        };

        if (indices.GraphicsFamily != indices.PresentFamily)
        {
            createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            createInfo.queueFamilyIndexCount = 2;
            createInfo.pQueueFamilyIndices = queueFamilyIndices;
        }
        else
        {
            createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
            createInfo.queueFamilyIndexCount = 0;     // Optional
            createInfo.pQueueFamilyIndices = nullptr; // Optional
        }

        createInfo.preTransform = support.Capabilities.currentTransform;
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        createInfo.presentMode = presentMode;
        createInfo.clipped = VK_TRUE;
        createInfo.oldSwapchain = VK_NULL_HANDLE;

        CheckVk(vkCreateSwapchainKHR(m_Device.Handle(), &createInfo, nullptr, &m_Swapchain), "create swapchain!");

        CheckVk(vkGetSwapchainImagesKHR(m_Device.Handle(), m_Swapchain, &imageCount, nullptr),
                "get swapchain images KHR (count)!");
        m_Images.resize(imageCount);

        CheckVk(vkGetSwapchainImagesKHR(m_Device.Handle(), m_Swapchain, &imageCount, m_Images.data()),
                "get swapchain images KHR (data!");

        m_ImageFormat = surfaceFormat.format;
        m_Extent = extent;
    }

    void Swapchain::CreateImageViews()
    {
        m_ImageViews.resize(m_Images.size());

        for (size_t i = 0; i < m_ImageViews.size(); i++)
        {
            VkImageViewCreateInfo createInfo{};
            createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            createInfo.image = m_Images[i];
            createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            createInfo.format = m_ImageFormat;

            createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

            createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            createInfo.subresourceRange.baseMipLevel = 0;
            createInfo.subresourceRange.levelCount = 1;
            createInfo.subresourceRange.baseArrayLayer = 0;
            createInfo.subresourceRange.layerCount = 1;

            CheckVk(vkCreateImageView(m_Device.Handle(), &createInfo, nullptr, &m_ImageViews[i]), "create image views!");
        }
    }

    VkSurfaceFormatKHR Swapchain::ChooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &formats) const
    {
        for (const auto &available : formats)
        {
            if (available.format == VK_FORMAT_B8G8R8A8_SRGB && available.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
                return available;
        }

        return formats[0];
    }

    VkPresentModeKHR Swapchain::ChoosePresentMode(const std::vector<VkPresentModeKHR> &modes) const
    {
        for (const auto &available : modes)
        {
            if (available == VK_PRESENT_MODE_MAILBOX_KHR)
                return available;
        }

        return VK_PRESENT_MODE_FIFO_KHR;
    }

    VkExtent2D Swapchain::ChooseExtent(const VkSurfaceCapabilitiesKHR &capabilities, const Window &window) const
    {
        if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
            return capabilities.currentExtent;

        int width, height;
        glfwGetFramebufferSize(window.Handle(), &width, &height);

        VkExtent2D actualExtent = {
            static_cast<uint32_t>(width),
            static_cast<uint32_t>(height),
        };

        actualExtent.width = std::clamp(actualExtent.width,
                                        capabilities.minImageExtent.width,
                                        capabilities.maxImageExtent.width);

        actualExtent.height = std::clamp(actualExtent.height,
                                         capabilities.minImageExtent.height,
                                         capabilities.maxImageExtent.height);

        return actualExtent;
    }

}
