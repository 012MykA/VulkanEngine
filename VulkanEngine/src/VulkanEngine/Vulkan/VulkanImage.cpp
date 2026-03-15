#include "VulkanImage.hpp"
#include "Debug/VulkanValidation.hpp"
#include "VulkanEngine/Core/Log.hpp"
#include "VulkanSingleTimeCommands.hpp"

#include <stdexcept>

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

    void VulkanImage::CopyFrom(VkBuffer srcBuffer, VkCommandPool commandPool, VkQueue graphicsQueue)
    {
        auto action = [&](VkCommandBuffer commandBuffer)
        {
            VkBufferImageCopy region{};
            region.bufferOffset = 0;
            region.bufferRowLength = 0;
            region.bufferImageHeight = 0;
            region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            region.imageSubresource.mipLevel = 0;
            region.imageSubresource.baseArrayLayer = 0;
            region.imageSubresource.layerCount = 1;
            region.imageOffset = {0, 0, 0};
            region.imageExtent = {m_Extent.width, m_Extent.height, 1};

            vkCmdCopyBufferToImage(commandBuffer, srcBuffer, m_Image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
        };
        VulkanSingleTimeCommands::Submit(action, commandPool, m_Device, graphicsQueue);
    }

    void VulkanImage::TransitionImageLayout(VkImageLayout newLayout, VkCommandPool commandPool, VkQueue graphicsQueue)
    {
        auto action = [&](VkCommandBuffer commandBuffer)
        {
            VkImageMemoryBarrier barrier{};
            barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            barrier.oldLayout = m_ImageLayout;
            barrier.newLayout = newLayout;
            barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.image = m_Image;
            barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            barrier.subresourceRange.baseMipLevel = 0;
            barrier.subresourceRange.levelCount = 1;
            barrier.subresourceRange.baseArrayLayer = 0;
            barrier.subresourceRange.layerCount = 1;

            VkPipelineStageFlags sourceStage;
            VkPipelineStageFlags destinationStage;

            if (m_ImageLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
            {
                barrier.srcAccessMask = 0;
                barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

                sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
                destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            }
            else if (m_ImageLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
            {
                barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

                sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
                destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            }
            else
            {
                throw std::invalid_argument("unsupported layout transition!");
            }

            vkCmdPipelineBarrier(
                commandBuffer,
                sourceStage, destinationStage,
                0,
                0, nullptr,
                0, nullptr,
                1, &barrier);
        };

        VulkanSingleTimeCommands::Submit(action, commandPool, m_Device, graphicsQueue);

        m_ImageLayout = newLayout;
    }

    VkMemoryRequirements VulkanImage::GetMemoryRequirements() const
    {
        VkMemoryRequirements requirements;
        vkGetImageMemoryRequirements(m_Device, m_Image, &requirements);
        return requirements;
    }

} // namespace ve
