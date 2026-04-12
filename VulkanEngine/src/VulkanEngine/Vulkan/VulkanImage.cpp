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
                             VkImage existingImage,
                             VkFormat format,
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
        CreateImageView(m_Aspect, VK_IMAGE_VIEW_TYPE_2D);
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
            .maxLod = (desc.maxLod == VK_LOD_CLAMP_NONE) ? static_cast<float>(m_MipLevels) : desc.maxLod,
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

    void VulkanImage::TransitionLayout(
        VkCommandBuffer cmd,
        VkImageLayout oldLayout,
        VkImageLayout newLayout,
        VkPipelineStageFlags srcStage,
        VkPipelineStageFlags dstStage,
        VkAccessFlags srcAccess,
        VkAccessFlags dstAccess)
    {
        VkImageMemoryBarrier barrier{
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
            .srcAccessMask = srcAccess,
            .dstAccessMask = dstAccess,
            .oldLayout = oldLayout,
            .newLayout = newLayout,
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .image = m_Image,
            .subresourceRange{
                .aspectMask = m_Aspect,
                .baseMipLevel = 0,
                .levelCount = m_MipLevels,
                .baseArrayLayer = 0,
                .layerCount = m_ArrayLayers,
            },
        };

        vkCmdPipelineBarrier(
            cmd,
            srcStage, dstStage,
            0,
            0, nullptr,
            0, nullptr,
            1, &barrier);
    }

    void VulkanImage::TransitionToShaderRead(VkCommandBuffer cmd)
    {
        TransitionLayout(
            cmd,
            m_CurrentLayout,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
            VK_ACCESS_TRANSFER_WRITE_BIT,
            VK_ACCESS_SHADER_READ_BIT);

        m_CurrentLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    }

    void VulkanImage::TransitionToColorAttachment(VkCommandBuffer cmd)
    {
        TransitionLayout(
            cmd,
            m_CurrentLayout,
            VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            0,
            VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT);

        m_CurrentLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    }

    void VulkanImage::TransitionToDepthAttachment(VkCommandBuffer cmd)
    {
        TransitionLayout(
            cmd,
            m_CurrentLayout,
            VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
            VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
            VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
            0,
            VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
                VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT);

        m_CurrentLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    }

    void VulkanImage::TransitionToTransferDst(VkCommandBuffer cmd)
    {
        TransitionLayout(
            cmd,
            m_CurrentLayout,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            0,
            VK_ACCESS_TRANSFER_WRITE_BIT);

        m_CurrentLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    }

    void VulkanImage::TransitionToTransferSrc(VkCommandBuffer cmd)
    {
        TransitionLayout(
            cmd,
            m_CurrentLayout,
            VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            VK_ACCESS_TRANSFER_WRITE_BIT,
            VK_ACCESS_TRANSFER_READ_BIT);

        m_CurrentLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    }

    void VulkanImage::CopyFromBuffer(VkCommandBuffer cmd, VkBuffer srcBuffer,
                                     uint32_t mipLevel, uint32_t layer) const
    {
        VkBufferImageCopy region{
            .bufferOffset = 0,
            .bufferRowLength = 0,
            .bufferImageHeight = 0,
            .imageSubresource{
                .aspectMask = m_Aspect,
                .mipLevel = mipLevel,
                .baseArrayLayer = layer,
                .layerCount = 1,
            },
            .imageOffset = {0, 0, 0},
            .imageExtent{
                .width = std::max(1u, m_Width >> mipLevel),
                .height = std::max(1u, m_Height >> mipLevel),
                .depth = 1,
            },
        };

        vkCmdCopyBufferToImage(
            cmd, srcBuffer, m_Image,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            1, &region);
    }

    void VulkanImage::CopyFromBufferAllLayers(VkCommandBuffer cmd, VkBuffer srcBuffer,
                                              VkDeviceSize faceByteSize) const
    {
        std::vector<VkBufferImageCopy> regions;
        regions.reserve(m_ArrayLayers);

        for (uint32_t layer = 0; layer < m_ArrayLayers; layer++)
        {
            regions.push_back(VkBufferImageCopy{
                .bufferOffset = layer * faceByteSize,
                .bufferRowLength = 0,
                .bufferImageHeight = 0,
                .imageSubresource{
                    .aspectMask = m_Aspect,
                    .mipLevel = 0,
                    .baseArrayLayer = layer,
                    .layerCount = 1,
                },
                .imageOffset = {0, 0, 0},
                .imageExtent = {m_Width, m_Height, 1},
            });
        }

        vkCmdCopyBufferToImage(
            cmd, srcBuffer, m_Image,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            static_cast<uint32_t>(regions.size()), regions.data());
    }

    void VulkanImage::GenerateMipmaps(VkCommandBuffer cmd)
    {
        if (m_MipLevels <= 1)
        {
            TransitionToShaderRead(cmd);
            return;
        }

        int32_t mipW = static_cast<int32_t>(m_Width);
        int32_t mipH = static_cast<int32_t>(m_Height);

        for (uint32_t i = 1; i < m_MipLevels; i++)
        {
            VkImageMemoryBarrier toSrc{
                .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
                .srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
                .dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT,
                .oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                .newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                .image = m_Image,
                .subresourceRange{
                    .aspectMask = m_Aspect,
                    .baseMipLevel = i - 1,
                    .levelCount = 1,
                    .baseArrayLayer = 0,
                    .layerCount = m_ArrayLayers,
                },
            };

            vkCmdPipelineBarrier(
                cmd,
                VK_PIPELINE_STAGE_TRANSFER_BIT,
                VK_PIPELINE_STAGE_TRANSFER_BIT,
                0,
                0, nullptr,
                0, nullptr,
                1, &toSrc);

            int32_t nextW = std::max(1, mipW / 2);
            int32_t nextH = std::max(1, mipH / 2);

            VkImageBlit blit{
                .srcSubresource{
                    .aspectMask = m_Aspect,
                    .mipLevel = i - 1,
                    .baseArrayLayer = 0,
                    .layerCount = m_ArrayLayers,
                },
                .dstSubresource{
                    .aspectMask = m_Aspect,
                    .mipLevel = i,
                    .baseArrayLayer = 0,
                    .layerCount = m_ArrayLayers,
                },
            };
            blit.srcOffsets[0] = {0, 0, 0};
            blit.srcOffsets[1] = {mipW, mipH, 1};
            blit.dstOffsets[0] = {0, 0, 0};
            blit.dstOffsets[1] = {nextW, nextH, 1};

            vkCmdBlitImage(
                cmd,
                m_Image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                m_Image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                1, &blit,
                VK_FILTER_LINEAR);

            mipW = nextW;
            mipH = nextH;
        }

        VkImageMemoryBarrier finalBarriers[2]{};

        finalBarriers[0].sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        finalBarriers[0].srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        finalBarriers[0].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        finalBarriers[0].oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        finalBarriers[0].newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        finalBarriers[0].srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        finalBarriers[0].dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        finalBarriers[0].image = m_Image;
        finalBarriers[0].subresourceRange.aspectMask = m_Aspect;
        finalBarriers[0].subresourceRange.baseMipLevel = 0;
        finalBarriers[0].subresourceRange.levelCount = m_MipLevels - 1;
        finalBarriers[0].subresourceRange.baseArrayLayer = 0;
        finalBarriers[0].subresourceRange.layerCount = m_ArrayLayers;

        finalBarriers[1].sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        finalBarriers[1].srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        finalBarriers[1].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        finalBarriers[1].oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        finalBarriers[1].newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        finalBarriers[1].srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        finalBarriers[1].dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        finalBarriers[1].image = m_Image;
        finalBarriers[1].subresourceRange.aspectMask = m_Aspect;
        finalBarriers[1].subresourceRange.baseMipLevel = m_MipLevels - 1;
        finalBarriers[1].subresourceRange.levelCount = 1;
        finalBarriers[1].subresourceRange.baseArrayLayer = 0;
        finalBarriers[1].subresourceRange.layerCount = m_ArrayLayers;

        vkCmdPipelineBarrier(
            cmd,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
            0, 0, nullptr, 0, nullptr,
            2, finalBarriers);

        m_CurrentLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    }

    VkDescriptorImageInfo VulkanImage::GetDescriptorInfo() const
    {
        assert(m_Sampler != VK_NULL_HANDLE && m_ImageView != VK_NULL_HANDLE);
        
        VkDescriptorImageInfo info{
            .sampler = m_Sampler,
            .imageView = m_ImageView,
            .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        };

        return info;
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
            .extent{
                .width = desc.width,
                .height = desc.height,
                .depth = 1,
            },
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
            .subresourceRange{
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
        {
            usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
                     VK_IMAGE_USAGE_SAMPLED_BIT |
                     VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
            break;
        }

        case ImageType::DepthAttachment:
        case ImageType::DepthStencilAttachment:
        {
            usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT |
                     VK_IMAGE_USAGE_SAMPLED_BIT;
            break;
        }

        case ImageType::Texture2D:
        case ImageType::TextureCube:
        {
            usage |= VK_IMAGE_USAGE_SAMPLED_BIT |
                     VK_IMAGE_USAGE_TRANSFER_DST_BIT |
                     VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
            break;
        }

        case ImageType::SwapchainColor:
        {
            usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
                     VK_IMAGE_USAGE_TRANSFER_DST_BIT;
            break;
        }
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
