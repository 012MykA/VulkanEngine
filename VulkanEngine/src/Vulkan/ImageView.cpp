#include "ImageView.hpp"
#include "Device.hpp"
#include "Validation.hpp"

namespace VE
{
    ImageView::ImageView(const Device &device, VkImage image, VkFormat format, VkImageAspectFlags aspect)
        : m_Device(device)
    {
        VkImageViewCreateInfo viewInfo{};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = image;
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = format;
        viewInfo.subresourceRange.aspectMask = aspect;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;

        CheckVk(vkCreateImageView(m_Device.Handle(), &viewInfo, nullptr, &m_ImageView),
                "failed to create texture image view!");
    }

    ImageView::~ImageView()
    {
        vkDestroyImageView(m_Device.Handle(), m_ImageView, nullptr);
    }

}
