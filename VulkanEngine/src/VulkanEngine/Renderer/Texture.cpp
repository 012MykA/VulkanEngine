#include "Texture.hpp"
#include "VulkanEngine/Vulkan/VulkanLogicalDevice.hpp"
#include "VulkanEngine/Vulkan/VulkanImmediateSubmit.hpp"
#include "VulkanEngine/Vulkan/VulkanBuffer.hpp"
#include "VulkanEngine/Core/Log.hpp"

#include <stb_image.h>

#include <cmath>

namespace ve
{
    std::shared_ptr<Texture> Texture::LoadFromFile(
        const std::string &path,
        const TextureDesc &desc,
        const VulkanAllocator &allocator,
        const VulkanLogicalDevice &logicalDevice,
        const VulkanImmediateSubmit &upload)
    {
        bool isHDR = stbi_is_hdr(path.c_str());

        int w = 0, h = 0, channels = 0;

        if (isHDR)
        {
            float *pixels = stbi_loadf(path.c_str(), &w, &h, &channels, STBI_rgb_alpha);
            if (!pixels)
                throw std::runtime_error("Texture: failed to load HDR: " + path);

            auto tex = std::make_shared<Texture>();
            tex->m_Path = path;
            tex->UploadPixels(
                reinterpret_cast<const uint8_t *>(pixels),
                static_cast<uint32_t>(w),
                static_cast<uint32_t>(h),
                4, allocator, logicalDevice, upload);

            stbi_image_free(pixels);
            VE_CORE_TRACE("Texture loaded (HDR): {} ({}x{})", path, w, h);
            return tex;
        }

        stbi_uc *pixels = stbi_load(path.c_str(), &w, &h, &channels, STBI_rgb_alpha);
        if (!pixels)
            throw std::runtime_error("Texture: failed to load: " + path);

        auto tex = std::make_shared<Texture>();
        tex->m_Path = path;
        tex->UploadPixels(pixels,
                          static_cast<uint32_t>(w),
                          static_cast<uint32_t>(h),
                          4, allocator, logicalDevice, upload);

        stbi_image_free(pixels);
        VE_CORE_TRACE("Texture loaded: {} ({}x{})", path, w, h);
        return tex;
    }

    std::shared_ptr<Texture> Texture::LoadFromMemory(
        const uint8_t *data,
        size_t size,
        const TextureDesc &desc,
        const VulkanAllocator &allocator,
        const VulkanLogicalDevice &device,
        const VulkanImmediateSubmit &upload)
    {
        int w = 0, h = 0, channels = 0;
        stbi_uc *pixels = stbi_load_from_memory(data, static_cast<int>(size),
                                                &w, &h, &channels, STBI_rgb_alpha);

        if (!pixels)
            throw std::runtime_error("Texture: failed to load from memory ");

        auto tex = std::make_shared<Texture>();
        tex->UploadPixels(
            pixels,
            static_cast<uint32_t>(w),
            static_cast<uint32_t>(h),
            4, allocator, device, upload);

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
            .arrayLayers = 1,
            .format = ResolveVkFormat(desc.format),
            .type = ImageType::Texture2D,
        };

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
        tex->UploadPixels(pixels, 1, 1, 4, allocator, device, upload);
        return tex;
    }

    VkDescriptorImageInfo Texture::GetDescriptorInfo() const
    {
        return m_Image->GetDescriptorInfo();
    }

    void Texture::UploadPixels(
        const uint8_t *pixels,
        uint32_t width, uint32_t height, uint32_t channels,
        const VulkanAllocator &allocator,
        const VulkanLogicalDevice &logicalDevice,
        const VulkanImmediateSubmit &upload)
    {
        m_Width = width;
        m_Height = height;
        m_MipLevels = CalcMipLevels(width, height);

        const VkDeviceSize imageSize = static_cast<VkDeviceSize>(width) * height * channels;

        // Staging buffer
        VulkanBuffer staging(allocator, MakeStagingBufferDesc(imageSize));
        staging.Upload(pixels, imageSize);

        // m_Image создаётся ДО submit — иначе лямбда захватит nullptr
        ImageDesc imageDesc{
            .width = width,
            .height = height,
            .mipLevels = m_MipLevels,
            .arrayLayers = 1,
            .format = VK_FORMAT_R8G8B8A8_SRGB,
            .type = ImageType::Texture2D,
        };
        m_Image = std::make_unique<VulkanImage>(allocator, logicalDevice, imageDesc);

        upload.Submit([&](VkCommandBuffer cmd)
                      {
            m_Image->TransitionToTransferDst(cmd);
            m_Image->CopyFromBuffer(cmd, staging.GetVkHandle());
            m_Image->GenerateMipmaps(cmd); });

        m_Image->CreateSampler();
    }

    VkFormat Texture::ResolveVkFormat(TextureFormat format)
    {
        // clang-format off
        switch (format)
        {
        case TextureFormat::RGBA8_SRGB:     return VK_FORMAT_R8G8B8A8_SRGB;
        case TextureFormat::RGBA8_UNORM:    return VK_FORMAT_R8G8B8A8_UNORM;
        case TextureFormat::RGBA16_SFLOAT:  return VK_FORMAT_R16G16B16A16_SFLOAT;
        case TextureFormat::RG8_UNORM:      return VK_FORMAT_R8G8B8A8_UNORM;
        case TextureFormat::R8_UNORM:       return VK_FORMAT_R8G8B8A8_UNORM;

        default:                            throw std::runtime_error("Texture: unknown format");
        }
        // clang-format on
    }

    uint32_t Texture::CalcMipLevels(uint32_t w, uint32_t h)
    {
        return static_cast<uint32_t>(std::floor(std::log2(std::max(w, h)))) + 1;
    }

} // namespace ve
