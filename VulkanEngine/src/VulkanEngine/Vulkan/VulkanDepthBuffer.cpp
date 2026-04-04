#include "VulkanDepthBuffer.hpp"
#include "VulkanLogicalDevice.hpp"
#include "VulkanAllocator.hpp"
#include "VulkanPhysicalDevice.hpp"
#include "Debug/VulkanValidation.hpp"
#include "VulkanEngine/Core/Log.hpp"

namespace ve
{
    VulkanDepthBuffer::VulkanDepthBuffer(
        const VulkanLogicalDevice &logicalDevice,
        const VulkanPhysicalDevice &physicalDevice,
        const VulkanAllocator &allocator,
        uint32_t width, uint32_t height)
        : m_Device(&logicalDevice),
          m_Format(FindDepthFormat(physicalDevice.GetVkHandle()))
    {
        Create(allocator, width, height);
        VE_CORE_TRACE("VulkanDepthBuffer created  ({}x{}, format={})", width, height, string_VkFormat(m_Format));
    }

    VulkanDepthBuffer::~VulkanDepthBuffer() = default;

    void VulkanDepthBuffer::Recreate(const VulkanAllocator &allocator, uint32_t width, uint32_t height)
    {
        m_Image.reset();
        Create(allocator, width, height);
    }

    void VulkanDepthBuffer::Create(const VulkanAllocator &allocator, uint32_t width, uint32_t height)
    {
        m_Image = std::make_unique<VulkanImage>(
            allocator,
            *m_Device,
            ImageDesc{
                .width = width,
                .height = height,
                .format = m_Format,
                .type = ImageType::DepthAttachment,
            });
    }

    VkFormat VulkanDepthBuffer::FindDepthFormat(VkPhysicalDevice physicalDevice)
    {
        const VkFormat candidates[] = {
            VK_FORMAT_D32_SFLOAT,
            VK_FORMAT_D32_SFLOAT_S8_UINT,
            VK_FORMAT_D24_UNORM_S8_UINT,
        };

        for (VkFormat format : candidates)
        {
            VkFormatProperties props;
            vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &props);

            if (props.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
                return format;
        }

        throw std::runtime_error("VulkanDepthBuffer: no supported depth format");
    }

} // namespace ve
