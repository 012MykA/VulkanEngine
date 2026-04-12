#include "Texture.hpp"
#include "VulkanEngine/Vulkan/VulkanLogicalDevice.hpp"
#include "VulkanEngine/Vulkan/VulkanImmediateSubmit.hpp"
#include "VulkanEngine/Vulkan/VulkanBuffer.hpp"
#include "VulkanEngine/Core/Log.hpp"

#include <stb_image.h>

#include <cmath>
#include <algorithm>

namespace ve
{
    std::shared_ptr<Texture> Texture::LoadFromFile(
        const std::string &path,
        TextureDesc desc,
        const VulkanAllocator &allocator,
        const VulkanLogicalDevice &logicalDevice,
        const VulkanImmediateSubmit &upload)
    {
        int w, h, channels;
        void *pixels = nullptr;
        bool isHDR = stbi_is_hdr(path.c_str());

        if (isHDR)
        {
            pixels = stbi_loadf(path.c_str(), &w, &h, &channels, STBI_rgb_alpha);
            desc.format = TextureFormat::RGBA32_SFLOAT;
        }
        else
        {
            pixels = stbi_load(path.c_str(), &w, &h, &channels, STBI_rgb_alpha);
        }

        if (!pixels)
        {
            VE_CORE_ERROR("Failed to load texture: {0}", path);
            return nullptr;
        }

        auto tex = std::make_shared<Texture>();
        tex->m_Path = path;
        tex->InitializeAndUpload(pixels, (uint32_t)w, (uint32_t)h, desc, allocator, logicalDevice, upload);

        stbi_image_free(pixels);
        return tex;
    }

    std::shared_ptr<Texture> Texture::LoadFromMemory(
        const uint8_t *data,
        size_t size,
        TextureDesc &desc,
        const VulkanAllocator &allocator,
        const VulkanLogicalDevice &device,
        const VulkanImmediateSubmit &upload)
    {
        int w, h, channels;
        void *pixels = nullptr;

        if (stbi_is_hdr_from_memory(data, (int)size))
        {
            pixels = stbi_loadf_from_memory(data, (int)size, &w, &h, &channels, STBI_rgb_alpha);
            desc.format = TextureFormat::RGBA32_SFLOAT;
        }
        else
        {
            pixels = stbi_load_from_memory(data, (int)size, &w, &h, &channels, STBI_rgb_alpha);
        }

        if (!pixels)
            return nullptr;

        auto tex = std::make_shared<Texture>();
        tex->InitializeAndUpload(pixels, (uint32_t)w, (uint32_t)h, desc, allocator, device, upload);

        stbi_image_free(pixels);
        return tex;
    }

    std::shared_ptr<Texture> Texture::CreateEmpty(
        uint32_t width,
        uint32_t height,
        const TextureDesc &desc,
        const VulkanAllocator &allocator,
        const VulkanLogicalDevice &device)
    {
        auto tex = std::make_shared<Texture>();
        tex->m_Width = width;
        tex->m_Height = height;
        tex->m_MipLevels = desc.generateMips ? CalcMipLevels(width, height) : 1;

        ImageDesc imageDesc{
            .width = width,
            .height = height,
            .mipLevels = tex->m_MipLevels,
            .format = ResolveVkFormat(desc.format),
            .type = ImageType::Texture2D};

        tex->m_Image = std::make_unique<VulkanImage>(allocator, device, imageDesc);
        tex->m_Image->CreateSampler();
        return tex;
    }

    std::shared_ptr<Texture> Texture::CreateSolid(
        uint8_t r, uint8_t g, uint8_t b, uint8_t a,
        const VulkanAllocator &allocator,
        const VulkanLogicalDevice &device,
        const VulkanImmediateSubmit &upload)
    {
        uint8_t pixels[4] = {r, g, b, a};
        auto tex = std::make_shared<Texture>();

        TextureDesc desc{.format = TextureFormat::RGBA8_UNORM, .generateMips = false};
        tex->InitializeAndUpload(pixels, 1, 1, desc, allocator, device, upload);

        return tex;
    }

    void Texture::InitializeAndUpload(
        const void *pixels,
        uint32_t width,
        uint32_t height,
        const TextureDesc &desc,
        const VulkanAllocator &allocator,
        const VulkanLogicalDevice &logicalDevice,
        const VulkanImmediateSubmit &upload)
    {
        m_Width = width;
        m_Height = height;
        m_MipLevels = desc.generateMips ? CalcMipLevels(width, height) : 1;

        size_t pixelSize = GetPixelSize(desc.format);
        VkDeviceSize dataSize = static_cast<VkDeviceSize>(width) * height * pixelSize;

        VulkanBuffer staging(allocator, MakeStagingBufferDesc(dataSize));
        staging.Upload(pixels, dataSize);

        ImageDesc imageDesc{
            .width = m_Width,
            .height = m_Height,
            .mipLevels = m_MipLevels,
            .format = ResolveVkFormat(desc.format),
            .type = ImageType::Texture2D,
        };
        m_Image = std::make_unique<VulkanImage>(allocator, logicalDevice, imageDesc);

        // clang-format off
        upload.Submit([&](VkCommandBuffer cmd) {
            m_Image->TransitionToTransferDst(cmd);
            m_Image->CopyFromBuffer(cmd, staging.GetVkHandle());

            if (m_MipLevels > 1)
                m_Image->GenerateMipmaps(cmd);
            else
                m_Image->TransitionToShaderRead(cmd);
        });
        // clang-format on

        m_Image->CreateSampler();
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
