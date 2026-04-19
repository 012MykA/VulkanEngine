#pragma once

#include "VulkanAllocator.hpp"

#include <vulkan/vulkan.h>

#include <cstdint>

namespace ve
{
    class VulkanLogicalDevice;

    enum class ImageType
    {
        ColorAttachment,
        DepthAttachment,
        DepthStencilAttachment,

        Texture2D,
        TextureCube,

        SwapchainColor,
    };

    struct ImageDesc
    {
        uint32_t width = 1;
        uint32_t height = 1;
        uint32_t mipLevels = 1;
        uint32_t arrayLayers = 1;
        VkFormat format = VK_FORMAT_UNDEFINED;
        ImageType type = ImageType::Texture2D;
        VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT;
        VkImageUsageFlags extraUsage = 0;
    };

    struct SamplerDesc
    {
        VkFilter magFilter = VK_FILTER_LINEAR;
        VkFilter minFilter = VK_FILTER_LINEAR;
        VkSamplerMipmapMode mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        VkSamplerAddressMode addressMode = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        float mipLodBias = 0.0f;
        float minLod = 0.0f;
        float maxLod = VK_LOD_CLAMP_NONE;
        float maxAnisotropy = 16.0f;
        VkBorderColor borderColor = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
        bool unnormalizedCoords = false;
    };

    class VulkanImage
    {
    public:
        VulkanImage(const VulkanAllocator &allocator,
                    const VulkanLogicalDevice &logicalDevice,
                    const ImageDesc &desc);

        // Constructor for swapchain image
        VulkanImage(const VulkanLogicalDevice &logicalDevice,
                    VkImage existingImage,
                    VkFormat format,
                    VkImageAspectFlags aspect);

        ~VulkanImage();

        VulkanImage(const VulkanImage &) = delete;
        VulkanImage &operator=(const VulkanImage &) = delete;

        VulkanImage(VulkanImage &&other) noexcept;
        VulkanImage &operator=(VulkanImage &&other) noexcept;

    public:
        void CreateSampler(const SamplerDesc &desc = {});
        void DestroySampler();

        void TransitionLayout(
            VkCommandBuffer cmd,
            VkImageLayout oldLayout,
            VkImageLayout newLayout,
            VkPipelineStageFlags srcStage,
            VkPipelineStageFlags dstStage,
            VkAccessFlags srcAccess,
            VkAccessFlags dstAccess);

        void TransitionToShaderRead(VkCommandBuffer cmd);
        void TransitionToColorAttachment(VkCommandBuffer cmd);
        void TransitionToDepthAttachment(VkCommandBuffer cmd);
        void TransitionToTransferDst(VkCommandBuffer cmd);
        void TransitionToTransferSrc(VkCommandBuffer cmd);

        void CopyFromBuffer(VkCommandBuffer cmd, VkBuffer srcBuffer,
                            uint32_t mipLevel = 0, uint32_t layer = 0) const;

        void CopyFromBufferAllLayers(VkCommandBuffer cmd, VkBuffer srcBuffer,
                                     VkDeviceSize faceByteSize) const;

        // Must be in TRANSFER_DST before generating mips
        // After it becomes SHADER_READ_ONLY_OPTIMAl
        void GenerateMipmaps(VkCommandBuffer cmd);

    public: // Getters
        VkImage GetVkHandle() const { return m_Image; }
        VkImageView GetView() const { return m_ImageView; }
        VkSampler GetSampler() const { return m_Sampler; }
        VkFormat GetFormat() const { return m_Format; }
        VkImageLayout GetLayout() const { return m_CurrentLayout; }
        uint32_t GetWidth() const { return m_Width; }
        uint32_t GetHeight() const { return m_Height; }
        uint32_t GetMipLevels() const { return m_MipLevels; }
        uint32_t GetArrayLayers() const { return m_ArrayLayers; }
        bool OwnsImage() const { return m_OwnsImage; }

        VkDescriptorImageInfo GetDescriptorInfo() const;

    private:
        void CreateImage(const VulkanAllocator &allocator, const ImageDesc &desc);
        void CreateImageView(VkImageAspectFlags aspect, VkImageViewType viewType);
        void Destroy();

        static VkImageUsageFlags ResolveUsageFlags(const ImageDesc &desc);
        static VkImageAspectFlags ResolveAspectFlags(VkFormat format);
        static VkImageViewType ResolveViewType(const ImageDesc &desc);

    private:
        VkDevice m_Device = VK_NULL_HANDLE;
        const VulkanAllocator *m_Allocator = nullptr;

        VkImage m_Image = VK_NULL_HANDLE;
        VkImageView m_ImageView = VK_NULL_HANDLE;
        VkSampler m_Sampler = VK_NULL_HANDLE;

        Allocation m_Allocation = {};
        VkFormat m_Format = VK_FORMAT_UNDEFINED;
        VkImageLayout m_CurrentLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        uint32_t m_Width = 0;
        uint32_t m_Height = 0;
        uint32_t m_MipLevels = 1;
        uint32_t m_ArrayLayers = 1;
        VkImageAspectFlags m_Aspect = VK_IMAGE_ASPECT_COLOR_BIT;
        bool m_OwnsImage = true;
    };

} // namespace ve
