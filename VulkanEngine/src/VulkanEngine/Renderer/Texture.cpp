#include "Texture.hpp"
#include "Backends/Vulkan/VulkanAllocator.hpp"
#include "Backends/Vulkan/VulkanLogicalDevice.hpp"
#include "Backends/Vulkan/VulkanImmediateSubmit.hpp"
#include "Backends/Vulkan/VulkanBuffer.hpp"

#include <algorithm>
#include <cmath>
#include <cassert>

namespace ve
{
    Texture::Texture(const TextureDesc &desc)
        : m_Width(desc.width), m_Height(desc.height),
          m_Format(desc.format), m_GenerateMips(desc.generateMips),
          m_MipLevels(m_GenerateMips ? CalcMipLevels(m_Width, m_Height) : 1)
    {
        if (desc.pixels)
        {
            SetData(desc.pixels, m_Width, m_Height, m_Format);
        }
    }

    void Texture::Upload(const VulkanAllocator &allocator,
                         const VulkanLogicalDevice &device,
                         const VulkanImmediateSubmit &upload,
                         const SamplerDesc &samplerDesc)
    {
        assert(!m_CPUData.empty() && "Nothing to upload");

        size_t pixelSize = GetPixelSize(m_Format);
        VkDeviceSize dataSize = static_cast<VkDeviceSize>(m_Width) * m_Height * pixelSize;

        VulkanBuffer staging(allocator, MakeStagingBufferDesc(dataSize));
        staging.Upload(m_CPUData.data(), dataSize);

        ImageDesc imageDesc{
            .width = m_Width,
            .height = m_Height,
            .mipLevels = m_MipLevels,
            .format = ResolveVkFormat(m_Format),
            .type = ImageType::Texture2D,
        };
        m_Image = std::make_unique<VulkanImage>(allocator, device, imageDesc);

        // clang-format off
        upload.Submit([&](VkCommandBuffer cmd)
        {
            m_Image->TransitionToTransferDst(cmd);

            m_Image->CopyFromBuffer(cmd, staging.GetVkHandle());

            if (m_GenerateMips)
            {
                m_Image->GenerateMipmaps(cmd);
            }
            else
            {
                m_Image->TransitionToShaderRead(cmd);
            }
        });
        // clang-format on

        m_Image->CreateSampler(samplerDesc);
    }

    void Texture::FreeCPUData()
    {
        m_CPUData.clear();
        m_CPUData.shrink_to_fit();
    }

    void Texture::SetData(const void *data, uint32_t width, uint32_t height, TextureFormat format)
    {
        m_Width = width;
        m_Height = height;
        m_Format = format;

        size_t size = static_cast<size_t>(width) * height * GetPixelSize(format);
        m_CPUData.assign(reinterpret_cast<const uint8_t *>(data), reinterpret_cast<const uint8_t *>(data) + size);
    }

    void Texture::SetGenerateMips(bool generate)
    {
        m_GenerateMips = generate;
        m_MipLevels = m_GenerateMips ? CalcMipLevels(m_Width, m_Height) : 1;
    }

    VkFormat Texture::ResolveVkFormat(TextureFormat format)
    {
        // clang-format off
        switch (format)
        {
        case TextureFormat::RGBA8_SRGB:         return VK_FORMAT_R8G8B8A8_SRGB;
        case TextureFormat::RGBA8_UNORM:        return VK_FORMAT_R8G8B8A8_UNORM;
        case TextureFormat::RGBA32_SFLOAT:      return VK_FORMAT_R32G32B32A32_SFLOAT;
        case TextureFormat::RGBA16_SFLOAT:      return VK_FORMAT_R16G16B16A16_SFLOAT;
        case TextureFormat::RG8_UNORM:          return VK_FORMAT_R8G8_UNORM;
        case TextureFormat::R8_UNORM:           return VK_FORMAT_R8_UNORM;
        default:                                return VK_FORMAT_UNDEFINED;
        }
        // clang-format on
    }

    size_t Texture::GetPixelSize(TextureFormat format)
    {
        // clang-format off
        switch (format)
        {
        case TextureFormat::RGBA32_SFLOAT:          return 16;
        case TextureFormat::RGBA16_SFLOAT:          return 8;
        case TextureFormat::RGBA8_SRGB:             return 4;
        case TextureFormat::RGBA8_UNORM:            return 4;        
        case TextureFormat::RG8_UNORM:              return 2;
        case TextureFormat::R8_UNORM:               return 1;

        default:                                    return 4;
        }
        // clang-format on
    }

    uint32_t Texture::CalcMipLevels(uint32_t w, uint32_t h)
    {
        return static_cast<uint32_t>(std::floor(std::log2(std::max(w, h)))) + 1;
    }

} // namespace ve
