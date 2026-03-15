#include "VulkanImageView.hpp"
#include "Debug/VulkanValidation.hpp"

namespace ve
{
    VulkanImageView::VulkanImageView(
        VkDevice device, const VulkanImage &image, VkImageAspectFlags aspect) : m_Device(device)
    {
        VkImageViewCreateInfo viewInfo{
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .image = image.GetImage(),
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format = image.GetFormat(),
            .components{
                .r = VK_COMPONENT_SWIZZLE_IDENTITY,
                .g = VK_COMPONENT_SWIZZLE_IDENTITY,
                .b = VK_COMPONENT_SWIZZLE_IDENTITY,
                .a = VK_COMPONENT_SWIZZLE_IDENTITY,
            },
            .subresourceRange{
                .aspectMask = aspect,
                .baseMipLevel = 0,
                .levelCount = image.GetMipLevels(),
                .baseArrayLayer = 0,
                .layerCount = 1,
            },
        };

        VkResult result = vkCreateImageView(m_Device, &viewInfo, nullptr, &m_ImageView);
        CHECK_VK_RESULT(result);
    }

    VulkanImageView::~VulkanImageView()
    {
        vkDestroyImageView(m_Device, m_ImageView, nullptr);
    }

} // namespace ve
