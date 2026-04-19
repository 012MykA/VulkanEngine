#include "Texture.hpp"
#include "Backends/Vulkan/VulkanLogicalDevice.hpp"
#include "Backends/Vulkan/VulkanImmediateSubmit.hpp"
#include "Backends/Vulkan/VulkanBuffer.hpp"
#include "VulkanEngine/Core/Timer.hpp"
#include "VulkanEngine/Core/Log.hpp"

#include <stb_image.h>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <cmath>
#include <algorithm>
#include <future>

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
            VE_CORE_ERROR("Failed to load texture: {}", path);
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
        Timer timer;

        // --- Loading an HDR panorama ---
        int srcW, srcH, ch;
        float *src = stbi_loadf(path.c_str(), &srcW, &srcH, &ch, STBI_rgb_alpha);
        if (!src)
        {
            VE_CORE_ERROR("LoadCubemapFromEquirect: failed to load '{}'", path);
            return nullptr;
        }

        // Axis table for 6 faces: +X -X +Y -Y +Z -Z
        // dir — viewing direction from the cube's center to the face
        // right — horizontal axis of the face (u increases to the right)
        // up — vertical axis of the face (v increases up)
        struct FaceAxes
        {
            glm::vec3 dir;
            glm::vec3 right;
            glm::vec3 up;
        };
        constexpr std::array<FaceAxes, 6> k_Faces{{
            {{1, 0, 0}, {0, 0, -1}, {0, -1, 0}},  // +X
            {{-1, 0, 0}, {0, 0, 1}, {0, -1, 0}},  // -X
            {{0, 1, 0}, {1, 0, 0}, {0, 0, 1}},    // +Y
            {{0, -1, 0}, {1, 0, 0}, {0, 0, -1}},  // -Y
            {{0, 0, 1}, {1, 0, 0}, {0, -1, 0}},   // +Z
            {{0, 0, -1}, {-1, 0, 0}, {0, -1, 0}}, // -Z
        }};

        const uint32_t N = faceSize;
        const uint32_t pixelsPerFace = N * N;
        std::vector<float> cubeData(6 * pixelsPerFace * 4);

        std::vector<std::future<void>> jobs;
        jobs.reserve(6);

        for (uint32_t face = 0; face < 6; face++)
        {
            // clang-format off
            jobs.push_back(std::async(std::launch::async, [&, face]() {
                const FaceAxes &ax = k_Faces[face];
                float *facePtr = cubeData.data() + face * pixelsPerFace * 4;

                const float invN = 1.0f / static_cast<float>(N);
                const float invW = 1.0f / static_cast<float>(srcW - 1);
                const float invH = 1.0f / static_cast<float>(srcH - 1);
                const float inv2Pi = 1.0f / (2.0f * glm::pi<float>());
                const float invPi  = 1.0f / glm::pi<float>();

                for (uint32_t y = 0; y < N; y++)
                {
                    // u/v from -1 to 1, pixel center
                    const float v = (2.0f * (y + 0.5f) * invN) - 1.0f;

                    for (uint32_t x = 0; x < N; x++)
                    {
                        const float u = (2.0f * (x + 0.5f) * invN) - 1.0f;

                        const glm::vec3 dir = glm::normalize(
                            ax.dir + u * ax.right + v * ax.up);

                        // Spherical -> equirectangular UV
                        // phi = [-pi, pi], theta = [-pi/2, pi/2]
                        const float phi   = std::atan2(dir.z, dir.x);
                        const float theta = std::asin(glm::clamp(dir.y, -1.0f, 1.0f));

                        const float su = (phi * inv2Pi) + 0.5f;
                        const float sv = (theta * invPi) + 0.5f;

                        // Bilinear sample
                        const float px = glm::clamp(su, 0.0f, 1.0f) * (srcW - 1);
                        const float py = glm::clamp(sv, 0.0f, 1.0f) * (srcH - 1);

                        const int x0 = static_cast<int>(px);
                        const int y0 = static_cast<int>(py);
                        const int x1 = std::min(x0 + 1, srcW - 1);
                        const int y1 = std::min(y0 + 1, srcH - 1);

                        const float fx = px - static_cast<float>(x0);
                        const float fy = py - static_cast<float>(y0);
                        const float wx = 1.0f - fx;
                        const float wy = 1.0f - fy;

                        const float *s00 = src + (y0 * srcW + x0) * 4;
                        const float *s10 = src + (y0 * srcW + x1) * 4;
                        const float *s01 = src + (y1 * srcW + x0) * 4;
                        const float *s11 = src + (y1 * srcW + x1) * 4;

                        float *dst = facePtr + (y * N + x) * 4;
                        for (int c = 0; c < 4; c++)
                        {
                            dst[c] = s00[c] * wx * wy
                                   + s10[c] * fx * wy
                                   + s01[c] * wx * fy
                                   + s11[c] * fx * fy;
                        }
                    }
                }
            }));
            // clang-format on
        }

        for (auto &j : jobs)
            j.get();
        stbi_image_free(src);

        // GPU Upload
        auto tex = std::make_shared<Texture>();
        tex->m_Width = N;
        tex->m_Height = N;
        tex->m_MipLevels = 1;
        tex->m_Path = path;

        const VkDeviceSize faceBytes = static_cast<VkDeviceSize>(pixelsPerFace) * 4 * sizeof(float);
        const VkDeviceSize totalBytes = 6 * faceBytes;

        VulkanBuffer staging(allocator, MakeStagingBufferDesc(totalBytes));
        staging.Upload(cubeData.data(), totalBytes);

        cubeData.clear();
        cubeData.shrink_to_fit();

        ImageDesc imageDesc{
            .width = N,
            .height = N,
            .mipLevels = 1,
            .arrayLayers = 6,
            .format = VK_FORMAT_R32G32B32A32_SFLOAT,
            .type = ImageType::TextureCube,
        };
        tex->m_Image = std::make_unique<VulkanImage>(allocator, logicalDevice, imageDesc);

        // clang-format off
        upload.Submit([&](VkCommandBuffer cmd) {
            tex->m_Image->TransitionToTransferDst(cmd);
            tex->m_Image->CopyFromBufferAllLayers(cmd, staging.GetVkHandle(), faceBytes);
            tex->m_Image->TransitionToShaderRead(cmd);
        });
        // clang-format on

        tex->m_Image->CreateSampler(SamplerDesc{
            .addressMode = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
        });

        std::string filename = std::filesystem::path(path).filename().string();
        VE_CORE_INFO("Cubemap '{}' loaded ({} ms)", filename, timer.ElapsedMilliseconds());
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
