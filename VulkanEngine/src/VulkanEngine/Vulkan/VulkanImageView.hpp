#pragma once

#include "VulkanImage.hpp"

#include <vulkan/vulkan.h>

namespace ve
{
    class VulkanImageView
    {
    public:
        VulkanImageView(VkDevice device, const VulkanImage &image, VkImageAspectFlags aspect);
        ~VulkanImageView();

        VkImageView GetImageView() const { return m_ImageView; }

    private:
        VkDevice m_Device;
        VkImageView m_ImageView = VK_NULL_HANDLE;
    };

} // namespace ve
