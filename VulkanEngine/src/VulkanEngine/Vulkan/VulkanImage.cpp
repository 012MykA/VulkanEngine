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
        VkImageCreateInfo imageInfo{
            .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
            .flags = 0, // Optional
            .imageType = VK_IMAGE_TYPE_2D,
            .format = m_Format,
            .extent{
                .width = extent.width,
                .height = extent.height,
                .depth = 1,
            },
            .mipLevels = 1,
            .arrayLayers = 1,
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .tiling = tiling,
            .usage = usage,
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
            .initialLayout = m_ImageLayout,
        };

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
            VkBufferImageCopy region{
                .bufferOffset = 0,
                .bufferRowLength = 0,
                .bufferImageHeight = 0,
                .imageSubresource{
                    .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                    .mipLevel = 0,
                    .baseArrayLayer = 0,
                    .layerCount = 1,
                },
                .imageOffset = {0, 0, 0},
                .imageExtent = {m_Extent.width, m_Extent.height, 1},
            };

            vkCmdCopyBufferToImage(commandBuffer, srcBuffer, m_Image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
        };
        VulkanSingleTimeCommands::Submit(action, commandPool, m_Device, graphicsQueue);
    }

    void VulkanImage::TransitionImageLayout(VkImageLayout newLayout, VkCommandPool commandPool, VkQueue graphicsQueue)
    {
        auto action = [&](VkCommandBuffer commandBuffer)
        {
            VkImageMemoryBarrier barrier{
                .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
                .oldLayout = m_ImageLayout,
                .newLayout = newLayout,
                .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                .image = m_Image,
                .subresourceRange{
                    .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                    .baseMipLevel = 0,
                    .levelCount = 1,
                    .baseArrayLayer = 0,
                    .layerCount = 1,
                },
            };

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
