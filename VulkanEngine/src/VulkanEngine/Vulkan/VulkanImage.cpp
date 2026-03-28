#include "VulkanImage.hpp"
#include "VulkanLogicalDevice.hpp"
#include "Debug/VulkanValidation.hpp"

namespace ve
{
    VulkanImage::VulkanImage(const VulkanAllocator &allocator,
                             const VulkanLogicalDevice &logicalDevice,
                             const ImageDesc &desc)
        : m_Device(logicalDevice.GetVkHandle()),
          m_Allocator(&allocator),
          m_Format(desc.format),
          m_Width(desc.width),
          m_Height(desc.height),
          m_MipLevels(desc.mipLevels),
          m_ArrayLayers(desc.arrayLayers),
          m_Aspect(ResolveAspectFlags(desc.format)),
          m_OwnsImage(true)
    {
        CreateImage(allocator, desc);
        CreateImageView(m_Aspect, ResolveViewType(desc));
    }

    VulkanImage::VulkanImage(const VulkanLogicalDevice &logicalDevice,
                             VkImage existingImage, VkFormat format,
                             VkImageAspectFlags aspect)
        : m_Device(logicalDevice.GetVkHandle()),
          m_Allocator(nullptr),
          m_Image(existingImage),
          m_Format(format),
          m_Width(0),
          m_Height(0),
          m_MipLevels(1),
          m_ArrayLayers(1),
          m_Aspect(aspect),
          m_OwnsImage(false)
    {
    }

    VulkanImage::~VulkanImage()
    {
        Destroy();
    }

    VulkanImage::VulkanImage(VulkanImage &&other) noexcept
        : m_Device(other.m_Device),
          m_Allocator(other.m_Allocator),
          m_Image(other.m_Image),
          m_ImageView(other.m_ImageView),
          m_Sampler(other.m_Sampler),
          m_Allocation(other.m_Allocation),
          m_Format(other.m_Format),
          m_CurrentLayout(other.m_CurrentLayout),
          m_Width(other.m_Width),
          m_Height(other.m_Height),
          m_MipLevels(other.m_MipLevels),
          m_ArrayLayers(other.m_ArrayLayers),
          m_Aspect(other.m_Aspect),
          m_OwnsImage(other.m_OwnsImage)
    {
        other.m_Image = VK_NULL_HANDLE;
        other.m_ImageView = VK_NULL_HANDLE;
        other.m_Sampler = VK_NULL_HANDLE;
        other.m_Allocation.handle = VK_NULL_HANDLE;
        other.m_Allocator = nullptr;
    }

    VulkanImage &VulkanImage::operator=(VulkanImage &&other) noexcept
    {
        if (this != &other)
        {
            Destroy();
            m_Device = other.m_Device;
            m_Allocator = other.m_Allocator;
            m_Image = other.m_Image;
            m_ImageView = other.m_ImageView;
            m_Sampler = other.m_Sampler;
            m_Allocation = other.m_Allocation;
            m_Format = other.m_Format;
            m_CurrentLayout = other.m_CurrentLayout;
            m_Width = other.m_Width;
            m_Height = other.m_Height;
            m_MipLevels = other.m_MipLevels;
            m_ArrayLayers = other.m_ArrayLayers;
            m_Aspect = other.m_Aspect;
            m_OwnsImage = other.m_OwnsImage;

            other.m_Image = VK_NULL_HANDLE;
            other.m_ImageView = VK_NULL_HANDLE;
            other.m_Sampler = VK_NULL_HANDLE;
            other.m_Allocation.handle = VK_NULL_HANDLE;
            other.m_Allocator = nullptr;
        }
        return *this;
    }

    void VulkanImage::CreateSampler(const SamplerDesc &desc)
    {
        if (m_Sampler != VK_NULL_HANDLE)
            DestroySampler();

        VkSamplerCreateInfo samplerInfo{
            .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
            .magFilter = desc.magFilter,
            .minFilter = desc.minFilter,
            .mipmapMode = desc.mipmapMode,
            .addressModeU = desc.addressMode,
            .addressModeV = desc.addressMode,
            .addressModeW = desc.addressMode,
            .mipLodBias = desc.mipLodBias,
            .anisotropyEnable = (desc.maxAnisotropy > 1.0f) ? VK_TRUE : VK_FALSE,
            .maxAnisotropy = desc.maxAnisotropy,
            .compareEnable = VK_FALSE,
            .compareOp = VK_COMPARE_OP_ALWAYS,
            .minLod = desc.minLod,
            .maxLod = (desc.maxLod == VK_LOD_CLAMP_NONE)
                          ? static_cast<float>(m_MipLevels)
                          : desc.maxLod,
            .borderColor = desc.borderColor,
            .unnormalizedCoordinates = desc.unnormalizedCoords ? VK_TRUE : VK_FALSE,
        };

        VkResult result = vkCreateSampler(m_Device, &samplerInfo, nullptr, &m_Sampler);
        CHECK_VK_RESULT(result);
    }

    void VulkanImage::DestroySampler()
    {
        if (m_Sampler != VK_NULL_HANDLE)
        {
            vkDestroySampler(m_Device, m_Sampler, nullptr);
            m_Sampler = VK_NULL_HANDLE;
        }
    }

    void VulkanImage::TransitionLayout(VkCommandBuffer cmd, VkImageLayout oldLayout, VkImageLayout newLayout, VkPipelineStageFlags2 srcStage, VkPipelineStageFlags2 dstStage) const
    {
        VkImageAspectFlags aspect = m_Aspect;

        VkImageMemoryBarrier2 barrier{
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
            .srcStageMask = srcStage,
            .srcAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT,
            .dstStageMask = dstStage,
            .dstAccessMask = VK_ACCESS_2_MEMORY_READ_BIT | VK_ACCESS_2_MEMORY_WRITE_BIT,
            .oldLayout = oldLayout,
            .newLayout = newLayout,
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .image = m_Image,
            .subresourceRange = {
                .aspectMask = aspect,
                .baseMipLevel = 0,
                .levelCount = m_MipLevels,
                .baseArrayLayer = 0,
                .layerCount = m_ArrayLayers,
            },
        };

        VkDependencyInfo depInfo{
            .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
            .imageMemoryBarrierCount = 1,
            .pImageMemoryBarriers = &barrier,
        };

        vkCmdPipelineBarrier2(cmd, &depInfo);
    }

    void VulkanImage::TransitionToShaderRead(VkCommandBuffer cmd) const
    {
        TransitionLayout(cmd, m_CurrentLayout,
                         VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                         VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
                         VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT);
    }

    void VulkanImage::TransitionToColorAttachment(VkCommandBuffer cmd) const
    {
        TransitionLayout(cmd, m_CurrentLayout,
                         VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                         VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
                         VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT);
    }

    void VulkanImage::TransitionToDepthAttachment(VkCommandBuffer cmd) const
    {
        TransitionLayout(cmd, m_CurrentLayout,
                         VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                         VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
                         VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT);
    }

    void VulkanImage::TransitionToTransferDst(VkCommandBuffer cmd) const
    {
        TransitionLayout(cmd, m_CurrentLayout,
                         VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                         VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
                         VK_PIPELINE_STAGE_2_TRANSFER_BIT);
    }

    void VulkanImage::TransitionToTransferSrc(VkCommandBuffer cmd) const
    {
        TransitionLayout(cmd, m_CurrentLayout,
                         VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                         VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
                         VK_PIPELINE_STAGE_2_TRANSFER_BIT);
    }

    void VulkanImage::CopyFromBuffer(VkCommandBuffer cmd, VkBuffer srcBuffer,
                                     uint32_t mipLevel, uint32_t layer) const
    {
        VkBufferImageCopy region{
            .bufferOffset = 0,
            .bufferRowLength = 0,
            .bufferImageHeight = 0,
            .imageSubresource = {
                .aspectMask = m_Aspect,
                .mipLevel = mipLevel,
                .baseArrayLayer = layer,
                .layerCount = 1,
            },
            .imageOffset = {0, 0, 0},
            .imageExtent = {
                .width = std::max(1u, m_Width >> mipLevel),
                .height = std::max(1u, m_Height >> mipLevel),
                .depth = 1,
            },
        };

        vkCmdCopyBufferToImage(cmd, srcBuffer, m_Image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
    }

    void VulkanImage::GenerateMipmaps(VkCommandBuffer cmd)
    {
        if (m_MipLevels <= 1)
            return;

        int32_t mipWidth = static_cast<int32_t>(m_Width);
        int32_t mipHeight = static_cast<int32_t>(m_Height);

        for (uint32_t i = 1; i < m_MipLevels; i++)
        {
            VkImageMemoryBarrier2 barrierSrc{
                .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
                .srcStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT,
                .srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT,
                .dstStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT,
                .dstAccessMask = VK_ACCESS_2_TRANSFER_READ_BIT,
                .oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                .newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                .image = m_Image,
                .subresourceRange = {m_Aspect, i - 1, 1, 0, 1},
            };

            VkDependencyInfo dep{.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
                                 .imageMemoryBarrierCount = 1,
                                 .pImageMemoryBarriers = &barrierSrc};
            vkCmdPipelineBarrier2(cmd, &dep);

            int32_t nextW = std::max(1, mipWidth / 2);
            int32_t nextH = std::max(1, mipHeight / 2);

            VkImageBlit2 blit{
                .sType = VK_STRUCTURE_TYPE_IMAGE_BLIT_2,
                .srcSubresource = {m_Aspect, i - 1, 0, 1},
                .srcOffsets = {VkOffset3D{0, 0, 0}, VkOffset3D{mipWidth, mipHeight, 1}},
                .dstSubresource = {m_Aspect, i, 0, 1},
                .dstOffsets = {VkOffset3D{0, 0, 0}, VkOffset3D{nextW, nextH, 1}},
            };

            VkBlitImageInfo2 blitInfo{
                .sType = VK_STRUCTURE_TYPE_BLIT_IMAGE_INFO_2,
                .srcImage = m_Image,
                .srcImageLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                .dstImage = m_Image,
                .dstImageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                .regionCount = 1,
                .pRegions = &blit,
                .filter = VK_FILTER_LINEAR,
            };

            vkCmdBlitImage2(cmd, &blitInfo);

            mipWidth = nextW;
            mipHeight = nextH;
        }

        VkImageMemoryBarrier2 finalBarrier{
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
            .srcStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT,
            .srcAccessMask = VK_ACCESS_2_TRANSFER_READ_BIT | VK_ACCESS_2_TRANSFER_WRITE_BIT,
            .dstStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT,
            .dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT,
            .oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            .newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            .image = m_Image,
            .subresourceRange = {m_Aspect, 0, m_MipLevels, 0, m_ArrayLayers},
        };

        VkDependencyInfo dep{.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
                             .imageMemoryBarrierCount = 1,
                             .pImageMemoryBarriers = &finalBarrier};
        vkCmdPipelineBarrier2(cmd, &dep);
    }

    VkDescriptorImageInfo VulkanImage::GetDescriptorInfo() const
    {
        return VkDescriptorImageInfo{
            .sampler = m_Sampler,
            .imageView = m_ImageView,
            .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        };
    }

    void VulkanImage::CreateImage(const VulkanAllocator &allocator, const ImageDesc &desc)
    {
        VkImageCreateFlags flags = 0;
        if (desc.type == ImageType::TextureCube)
            flags |= VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;

        VkImageCreateInfo imageInfo{
            .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
            .flags = flags,
            .imageType = VK_IMAGE_TYPE_2D,
            .format = desc.format,
            .extent = {desc.width, desc.height, 1},
            .mipLevels = desc.mipLevels,
            .arrayLayers = desc.arrayLayers,
            .samples = desc.samples,
            .tiling = VK_IMAGE_TILING_OPTIMAL,
            .usage = ResolveUsageFlags(desc),
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        };

        AllocationDesc allocDesc{.gpuOnly = true};
        m_Allocation = allocator.AllocateImage(imageInfo, allocDesc, m_Image);
    }

    void VulkanImage::CreateImageView(VkImageAspectFlags aspect, VkImageViewType viewType)
    {
        VkImageViewCreateInfo viewInfo{
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .image = m_Image,
            .viewType = viewType,
            .format = m_Format,
            .subresourceRange = {
                .aspectMask = aspect,
                .baseMipLevel = 0,
                .levelCount = m_MipLevels,
                .baseArrayLayer = 0,
                .layerCount = m_ArrayLayers,
            },
        };

        VkResult result = vkCreateImageView(m_Device, &viewInfo, nullptr, &m_ImageView);
        CHECK_VK_RESULT(result);
    }

    void VulkanImage::Destroy()
    {
        DestroySampler();

        if (m_ImageView != VK_NULL_HANDLE)
        {
            vkDestroyImageView(m_Device, m_ImageView, nullptr);
            m_ImageView = VK_NULL_HANDLE;
        }

        if (m_OwnsImage && m_Image != VK_NULL_HANDLE && m_Allocator)
        {
            m_Allocator->FreeImage(m_Image, m_Allocation);
            m_Image = VK_NULL_HANDLE;
            m_Allocation.handle = VK_NULL_HANDLE;
        }
    }

    VkImageUsageFlags VulkanImage::ResolveUsageFlags(const ImageDesc &desc)
    {
        VkImageUsageFlags usage = desc.extraUsage;

        switch (desc.type)
        {
        case ImageType::ColorAttachment:
            usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
                     VK_IMAGE_USAGE_SAMPLED_BIT |
                     VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
            break;

        case ImageType::DepthAttachment:
        case ImageType::DepthStencilAttachment:
            usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT |
                     VK_IMAGE_USAGE_SAMPLED_BIT;
            break;

        case ImageType::Texture2D:
        case ImageType::TextureCube:
            usage |= VK_IMAGE_USAGE_SAMPLED_BIT |
                     VK_IMAGE_USAGE_TRANSFER_DST_BIT |
                     VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
            break;

        case ImageType::SwapchainColor:
            usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
                     VK_IMAGE_USAGE_TRANSFER_DST_BIT;
            break;
        }

        return usage;
    }

    VkImageAspectFlags VulkanImage::ResolveAspectFlags(VkFormat format)
    {
        switch (format)
        {
        case VK_FORMAT_D32_SFLOAT:
            return VK_IMAGE_ASPECT_DEPTH_BIT;

        case VK_FORMAT_D24_UNORM_S8_UINT:
        case VK_FORMAT_D32_SFLOAT_S8_UINT:
            return VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;

        default:
            return VK_IMAGE_ASPECT_COLOR_BIT;
        }
    }

    VkImageViewType VulkanImage::ResolveViewType(const ImageDesc &desc)
    {
        if (desc.type == ImageType::TextureCube)
            return VK_IMAGE_VIEW_TYPE_CUBE;

        if (desc.arrayLayers > 1)
            return VK_IMAGE_VIEW_TYPE_2D_ARRAY;

        return VK_IMAGE_VIEW_TYPE_2D;
    }

} // namespace ve
