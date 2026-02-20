#include "DepthBuffer.hpp"
#include "CommandPool.hpp"
#include "Image.hpp"
#include "DeviceMemory.hpp"
#include "ImageView.hpp"
#include "Device.hpp"

#include <stdexcept>

namespace VE
{

    namespace
    {
        VkFormat FindSupportedFormat(const Device &device, const std::vector<VkFormat> &candidates, const VkImageTiling tiling, const VkFormatFeatureFlags features)
        {
            for (auto format : candidates)
            {
                VkFormatProperties props;
                vkGetPhysicalDeviceFormatProperties(device.GetPhysicalDevice(), format, &props);

                if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features)
                {
                    return format;
                }

                if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features)
                {
                    return format;
                }
            }

            throw std::runtime_error("failed to find supported format");
        }

        VkFormat FindDepthFormat(const Device &device)
        {
            return FindSupportedFormat(
                device,
                {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
                VK_IMAGE_TILING_OPTIMAL,
                VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
        }
    }

    DepthBuffer::DepthBuffer(const CommandPool &commandPool, const VkExtent2D extent)
        : m_Format(FindDepthFormat(commandPool.GetDevice()))
    {
        const auto &device = commandPool.GetDevice();

        m_Image = std::make_unique<Image>(device, extent, m_Format, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);
        m_ImageMemory = std::make_unique<DeviceMemory>(m_Image->AllocateMemory(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT));
        m_ImageView = std::make_unique<ImageView>(device, m_Image->Handle(), m_Format, VK_IMAGE_ASPECT_DEPTH_BIT);

        m_Image->TransitionImageLayout(commandPool, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
    }

    DepthBuffer::~DepthBuffer() = default;

}
