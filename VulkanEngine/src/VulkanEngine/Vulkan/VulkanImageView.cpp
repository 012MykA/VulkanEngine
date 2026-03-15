#include "VulkanImageView.hpp"
#include "Debug/VulkanValidation.hpp"

namespace ve
{
    VulkanImageView::VulkanImageView(
        VkDevice device, const VulkanImage &image, VkImageAspectFlags aspect) : m_Device(device)
    {
        VkImageViewCreateInfo viewInfo{};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = image.GetImage();
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = image.GetFormat();

        viewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        viewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        viewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        viewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

        viewInfo.subresourceRange.aspectMask = aspect;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = image.GetMipLevels();
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;

        VkResult result = vkCreateImageView(m_Device, &viewInfo, nullptr, &m_ImageView);
        CHECK_VK_RESULT(result);
    }

    VulkanImageView::~VulkanImageView()
    {
        vkDestroyImageView(m_Device, m_ImageView, nullptr);
    }

} // namespace ve
