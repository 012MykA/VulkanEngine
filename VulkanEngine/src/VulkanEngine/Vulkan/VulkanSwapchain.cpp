#include "VulkanSwapchain.hpp"
#include "VulkanPhysicalDevice.hpp"
#include "VulkanLogicalDevice.hpp"
#include "VulkanSurface.hpp"
#include "Debug/VulkanValidation.hpp"
#include "VulkanEngine/Core/Log.hpp"

#include <cassert>

namespace ve
{
    VulkanSwapchain::VulkanSwapchain(
        const VulkanPhysicalDevice &physicalDevice,
        const VulkanLogicalDevice &logicalDevice,
        const VulkanSurface &surface,
        const SwapchainDesc &desc)
        : m_PhysicalDevice(physicalDevice),
          m_LogicalDevice(logicalDevice),
          m_Surface(surface.GetVkHandle()),
          m_Desc(desc)
    {
        Create(desc.width, desc.height);
    }

    VulkanSwapchain::~VulkanSwapchain()
    {
        VkDevice device = m_LogicalDevice.GetVkHandle();

        for (auto imageView : m_ImageViews)
            vkDestroyImageView(device, imageView, nullptr);
        m_ImageViews.clear();

        if (m_Swapchain != VK_NULL_HANDLE)
        {
            vkDestroySwapchainKHR(device, m_Swapchain, nullptr);
            m_Swapchain = VK_NULL_HANDLE;
        }

        m_Images.clear();
    }

    void VulkanSwapchain::Recreate(uint32_t newWidth, uint32_t newHeight)
    {
        VkDevice device = m_LogicalDevice.GetVkHandle();

        VkSwapchainKHR oldSwapchain = m_Swapchain;
        m_Swapchain = VK_NULL_HANDLE;

        for (auto view : m_ImageViews)
            vkDestroyImageView(device, view, nullptr);
        m_ImageViews.clear();

        Create(newWidth, newHeight, oldSwapchain);

        if (oldSwapchain != VK_NULL_HANDLE)
            vkDestroySwapchainKHR(device, oldSwapchain, nullptr);
    }

    void VulkanSwapchain::Create(uint32_t width, uint32_t height, VkSwapchainKHR oldSwapchain)
    {
        const auto support = m_PhysicalDevice.QuerySwapchainSupport();
        const auto &indices = m_PhysicalDevice.GetQueueFamilies();

        VkSurfaceFormatKHR surfaceFormat = ChooseSurfaceFormat(support.formats);
        VkPresentModeKHR presentMode = ChoosePresentMode(support.presentModes);
        VkExtent2D extent = ChooseExtent(support.capabilities, width, height);

        uint32_t imageCount = support.capabilities.minImageCount + 1;
        if (support.capabilities.maxImageCount > 0)
            imageCount = std::min(imageCount, support.capabilities.maxImageCount);

        VkSwapchainCreateInfoKHR createInfo{
            .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
            .surface = m_Surface,
            .minImageCount = imageCount,
            .imageFormat = surfaceFormat.format,
            .imageColorSpace = surfaceFormat.colorSpace,
            .imageExtent = extent,
            .imageArrayLayers = 1,
            .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
            .preTransform = support.capabilities.currentTransform,
            .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
            .presentMode = presentMode,
            .clipped = VK_TRUE,
            .oldSwapchain = oldSwapchain,
        };

        uint32_t queueFamilies[] = {
            indices.graphicsFamily.value(),
            indices.presentFamily.value(),
        };

        if (indices.graphicsFamily != indices.presentFamily)
        {
            createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            createInfo.queueFamilyIndexCount = 2;
            createInfo.pQueueFamilyIndices = queueFamilies;
        }
        else
        {
            createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
            createInfo.queueFamilyIndexCount = 0;
            createInfo.pQueueFamilyIndices = nullptr;
        }

        VkResult result = vkCreateSwapchainKHR(m_LogicalDevice.GetVkHandle(), &createInfo, nullptr, &m_Swapchain);
        CHECK_VK_RESULT(result);

        uint32_t actualImageCount = 0;
        vkGetSwapchainImagesKHR(m_LogicalDevice.GetVkHandle(), m_Swapchain, &actualImageCount, nullptr);
        m_Images.resize(actualImageCount);
        vkGetSwapchainImagesKHR(m_LogicalDevice.GetVkHandle(), m_Swapchain, &actualImageCount, m_Images.data());

        m_Format = surfaceFormat.format;
        m_Extent = extent;

        CreateImageViews();

        VE_CORE_TRACE("Swapchain created:");
        VE_CORE_TRACE("  Extent: {}x{}", extent.width, extent.height);
        VE_CORE_TRACE("  Images: {}", actualImageCount);
        VE_CORE_TRACE("  Format: {}", string_VkFormat(m_Format));
        VE_CORE_TRACE("  Color space: {}", string_VkColorSpaceKHR(surfaceFormat.colorSpace));
        VE_CORE_TRACE("  Present mode: {}", string_VkPresentModeKHR(presentMode));
    }

    void VulkanSwapchain::CreateImageViews()
    {
        m_ImageViews.resize(m_Images.size());

        for (size_t i = 0; i < m_Images.size(); i++)
        {
            VkImageViewCreateInfo createInfo{
                .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
                .image = m_Images[i],
                .viewType = VK_IMAGE_VIEW_TYPE_2D,
                .format = m_Format,
                .components = {
                    .r = VK_COMPONENT_SWIZZLE_IDENTITY,
                    .g = VK_COMPONENT_SWIZZLE_IDENTITY,
                    .b = VK_COMPONENT_SWIZZLE_IDENTITY,
                    .a = VK_COMPONENT_SWIZZLE_IDENTITY,
                },
                .subresourceRange = {
                    .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                    .baseMipLevel = 0,
                    .levelCount = 1,
                    .baseArrayLayer = 0,
                    .layerCount = 1,
                },
            };

            VkResult result = vkCreateImageView(m_LogicalDevice.GetVkHandle(), &createInfo, nullptr, &m_ImageViews[i]);
            CHECK_VK_RESULT(result);
        }
    }

    VkSurfaceFormatKHR VulkanSwapchain::ChooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &available) const
    {
        assert(!available.empty() && "No surface formats available");

        for (const auto &preferred : m_Desc.preferredFormats)
            for (const auto &format : available)
                if (format.format == preferred.format &&
                    format.colorSpace == preferred.colorSpace)
                    return format;

        VE_CORE_WARN("Preferred surface format not found, using fallback");
        return available[0];
    }

    VkPresentModeKHR VulkanSwapchain::ChoosePresentMode(const std::vector<VkPresentModeKHR> &available) const
    {
        for (const auto &preferred : m_Desc.preferredPresentModes)
            for (const auto &mode : available)
                if (mode == preferred)
                    return mode;

        return VK_PRESENT_MODE_FIFO_KHR;
    }

    VkExtent2D VulkanSwapchain::ChooseExtent(const VkSurfaceCapabilitiesKHR &capabilities, uint32_t width, uint32_t height) const
    {
        if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
            return capabilities.currentExtent;

        return VkExtent2D{
            .width = std::clamp(width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width),
            .height = std::clamp(height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height),
        };
    }

} // namespace ve
