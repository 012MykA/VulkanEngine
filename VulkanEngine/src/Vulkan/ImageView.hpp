#pragma once

#include <vulkan/vulkan.h>

namespace VE
{
    class Device;

    class ImageView
    {
    public:
        ImageView(const Device &device, VkImage image, VkFormat format, VkImageAspectFlags aspect);
        ~ImageView();

        VkImageView Handle() const { return m_ImageView; }

    private:
        const Device &m_Device;
        VkImageView m_ImageView;
    };

}
