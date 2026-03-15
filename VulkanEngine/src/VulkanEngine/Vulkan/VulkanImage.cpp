#include "VulkanImage.hpp"
#include "Debug/VulkanValidation.hpp"
#include "VulkanEngine/Core/Log.hpp"

namespace ve
{
    VulkanImage::VulkanImage(VkDevice device, const VkExtent2D extent, const VkFormat format,
                             VkImageTiling tiling, VkImageUsageFlags usage, const std::string &debugName)
        : m_Device(device), m_Extent(extent), m_Format(format), m_ImageLayout(VK_IMAGE_LAYOUT_UNDEFINED), m_DebugName(debugName)
    {
        VkImageCreateInfo imageInfo{};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.extent.width = extent.width;
        imageInfo.extent.height = extent.height;
        imageInfo.extent.depth = 1;
        imageInfo.mipLevels = 1;
        imageInfo.arrayLayers = 1;
        imageInfo.format = m_Format;
        imageInfo.tiling = tiling;
        imageInfo.initialLayout = m_ImageLayout;
        imageInfo.usage = usage;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageInfo.flags = 0; // Optional

        VkResult result = vkCreateImage(m_Device, &imageInfo, nullptr, &m_Image);
        CHECK_VK_RESULT(result);
        VE_CORE_TRACE("VulkanImage ({0}) created", m_DebugName);
    }

    VulkanImage::~VulkanImage()
    {
        vkDestroyImage(m_Device, m_Image, nullptr);
        VE_CORE_TRACE("VulkanImage ({0}) destroyed", m_DebugName);
    }

    VulkanDeviceMemory VulkanImage::AllocateMemory(VkPhysicalDevice physicalDevice, const VkMemoryPropertyFlags properties) const
    {
        const auto requirements = GetMemoryRequirements();
        VulkanDeviceMemory memory(m_Device, physicalDevice, requirements.size, requirements.memoryTypeBits, 0, properties, m_DebugName);

        VkResult result = vkBindImageMemory(m_Device, m_Image, memory.Handle(), 0);
        CHECK_VK_RESULT(result);

        return memory;
    }

    VkMemoryRequirements VulkanImage::GetMemoryRequirements() const
    {
        VkMemoryRequirements requirements;
        vkGetImageMemoryRequirements(m_Device, m_Image, &requirements);
        return requirements;
    }

} // namespace ve
