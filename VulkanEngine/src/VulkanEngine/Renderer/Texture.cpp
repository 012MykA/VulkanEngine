#include "Texture.hpp"
#include "VulkanEngine/Vulkan/VulkanLogicalDevice.hpp"
#include "VulkanEngine/Vulkan/VulkanImmediateSubmit.hpp"
#include "VulkanEngine/Vulkan/VulkanBuffer.hpp"
#include "VulkanEngine/Core/Log.hpp"

#include <stb_image.h>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

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

    std::shared_ptr<Texture> Texture::LoadCubemapFromEquirect(
        const std::string &path,
        uint32_t faceSize,
        const VulkanAllocator &allocator,
        const VulkanLogicalDevice &logicalDevice,
        const VulkanImmediateSubmit &upload)
    {
        // clang-format off
        const glm::vec3 k_FaceDirs[6] = {
            { 1,  0,  0}, {-1,  0,  0},
            { 0,  1,  0}, { 0, -1,  0},
            { 0,  0,  1}, { 0,  0, -1},
        };
        const glm::vec3 k_FaceUps[6] = {
            { 0, -1,  0}, { 0, -1,  0},
            { 0,  0,  1}, { 0,  0, -1},
            { 0, -1,  0}, { 0, -1,  0},
        };
        // clang-format on

        int w, h, ch;
        float *src = stbi_loadf(path.c_str(), &w, &h, &ch, STBI_rgb_alpha);
        if (!src)
        {
            VE_CORE_ERROR("LoadCubemapFromEquirect: failed to load '{}'", path);
            return nullptr;
        }

        const uint32_t N = faceSize;
        std::vector<float> cubeData(6 * N * N * 4);

        for (uint32_t face = 0; face < 6; face++)
        {
            glm::vec3 dir = k_FaceDirs[face];
            glm::vec3 up = k_FaceUps[face];
            glm::vec3 right = glm::normalize(glm::cross(dir, up));
            up = glm::normalize(glm::cross(right, dir));

            float *facePtr = cubeData.data() + face * N * N * 4;

            for (uint32_t y = 0; y < N; y++)
            {
                for (uint32_t x = 0; x < N; x++)
                {
                    // UV от -1 до 1
                    float u = (2.0f * (x + 0.5f) / N) - 1.0f;
                    float v = (2.0f * (y + 0.5f) / N) - 1.0f;

                    glm::vec3 sample = glm::normalize(dir + u * right + v * up);

                    // Spherical mapping
                    float phi = std::atan2(sample.z, sample.x);
                    float theta = std::asin(glm::clamp(sample.y, -1.0f, 1.0f));

                    float su = (phi / (2.0f * glm::pi<float>())) + 0.5f;
                    float sv = (theta / glm::pi<float>()) + 0.5f;

                    su = glm::clamp(su, 0.0f, 1.0f);
                    sv = glm::clamp(sv, 0.0f, 1.0f);

                    float px = su * (w - 1);
                    float py = sv * (h - 1);

                    int x0 = (int)px, y0 = (int)py;
                    int x1 = std::min(x0 + 1, w - 1);
                    int y1 = std::min(y0 + 1, h - 1);

                    float fx = px - x0, fy = py - y0;

                    auto sample4 = [&](int sx, int sy) -> glm::vec4
                    {
                        const float *p = src + (sy * w + sx) * 4;
                        return {p[0], p[1], p[2], p[3]};
                    };

                    glm::vec4 col =
                        sample4(x0, y0) * (1 - fx) * (1 - fy) +
                        sample4(x1, y0) * fx * (1 - fy) +
                        sample4(x0, y1) * (1 - fx) * fy +
                        sample4(x1, y1) * fx * fy;

                    float *dst = facePtr + (y * N + x) * 4;
                    dst[0] = col.r;
                    dst[1] = col.g;
                    dst[2] = col.b;
                    dst[3] = col.a;
                }
            }
        }
        stbi_image_free(src);

        auto tex = std::make_shared<Texture>();
        tex->m_Width = N;
        tex->m_Height = N;
        tex->m_MipLevels = 1;
        tex->m_Path = path;

        VkDeviceSize faceBytes = N * N * 4 * sizeof(float);
        VkDeviceSize totalBytes = 6 * faceBytes;

        VulkanBuffer staging(allocator, MakeStagingBufferDesc(totalBytes));
        staging.Upload(cubeData.data(), totalBytes);

        ImageDesc imageDesc{
            .width = N,
            .height = N,
            .mipLevels = 1,
            .arrayLayers = 6,
            .format = VK_FORMAT_R32G32B32A32_SFLOAT,
            .type = ImageType::TextureCube,
        };
        tex->m_Image = std::make_unique<VulkanImage>(allocator, logicalDevice, imageDesc);

        upload.Submit([&](VkCommandBuffer cmd)
                      {
        tex->m_Image->TransitionToTransferDst(cmd);

        for (uint32_t face = 0; face < 6; face++)
        {
            VkBufferImageCopy region{
                .bufferOffset      = face * faceBytes,
                .bufferRowLength   = 0,
                .bufferImageHeight = 0,
                .imageSubresource{
                    .aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT,
                    .mipLevel       = 0,
                    .baseArrayLayer = face,
                    .layerCount     = 1,
                },
                .imageOffset = {0, 0, 0},
                .imageExtent = {N, N, 1},
            };
            vkCmdCopyBufferToImage(
                cmd,
                staging.GetVkHandle(),
                tex->m_Image->GetVkHandle(),
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                1, &region);
        }

        tex->m_Image->TransitionToShaderRead(cmd); });

        tex->m_Image->CreateSampler(SamplerDesc{
            .addressMode = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
        });

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
