#include "TextureLoader.hpp"
#include "VulkanEngine/Core/Log.hpp"

#include <stb_image.h>
#include <glm/glm.hpp>

#include <cstring>
#include <vector>
#include <filesystem>
#include <numbers>

namespace ve
{
    namespace
    {
        void FailedToLoadTexture(const std::string &path)
        {
            VE_CORE_ERROR("Failed to load texture '{}'", path);
        }

        glm::vec3 GetCubeVector(int face, float u, float v)
        {
            float uc = 2.0f * u - 1.0f;
            float vc = 2.0f * v - 1.0f;

            vc = -vc;

            switch (face)
            {
            case 0:
                return glm::normalize(glm::vec3(1.0f, vc, -uc)); // +X
            case 1:
                return glm::normalize(glm::vec3(-1.0f, vc, uc)); // -X
            case 2:
                return glm::normalize(glm::vec3(uc, 1.0f, -vc)); // +Y
            case 3:
                return glm::normalize(glm::vec3(uc, -1.0f, vc)); // -Y
            case 4:
                return glm::normalize(glm::vec3(uc, vc, 1.0f)); // +Z
            case 5:
                return glm::normalize(glm::vec3(-uc, vc, -1.0f)); // -Z
            }
            return glm::vec3(0.0f);
        }
    }

    std::shared_ptr<Texture> TextureLoader::CreateSolid(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
    {
        uint8_t pixels[4] = {r, g, b, a};

        TextureDesc desc{
            .pixels = pixels,
            .width = 1,
            .height = 1,
            .format = TextureFormat::RGBA8_UNORM,
            .generateMips = false,
        };

        return std::make_shared<Texture>(desc);
    }

    std::shared_ptr<Texture> TextureLoader::Load(const std::string &path)
    {
        int texWidth, texHeight, texChannels;
        void *pixels = nullptr;
        TextureFormat format = TextureFormat::RGBA8_SRGB;
        bool isHDR = stbi_is_hdr(path.c_str());

        if (isHDR)
        {
            pixels = stbi_loadf(path.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
            format = TextureFormat::RGBA32_SFLOAT;
        }
        else
        {
            pixels = stbi_load(path.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
            format = TextureFormat::RGBA8_SRGB;
        }

        if (!pixels)
        {
            FailedToLoadTexture(path);
            return nullptr;
        }

        TextureDesc desc{
            .pixels = pixels,
            .width = static_cast<uint32_t>(texWidth),
            .height = static_cast<uint32_t>(texHeight),
            .format = format,
        };
        auto texture = std::make_shared<Texture>(desc);

        stbi_image_free(pixels);

        std::string filename = std::filesystem::path(path).filename().string();
        VE_CORE_INFO("Texture{} '{}' loaded", isHDR ? "(HDR)" : "", filename);
        return texture;
    }

    std::shared_ptr<Texture> TextureLoader::LoadCubeMap(const std::array<std::string, 6> &paths)
    {
        int texWidth, texHeight, texChannels;
        std::vector<void *> facesData;
        TextureFormat format = TextureFormat::RGBA8_SRGB;
        bool isHDR = stbi_is_hdr(paths[0].c_str());

        for (const auto &path : paths)
        {
            void *pixels = nullptr;
            if (isHDR)
            {
                pixels = stbi_loadf(path.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
                format = TextureFormat::RGBA32_SFLOAT;
            }
            else
            {
                pixels = stbi_load(path.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
                format = TextureFormat::RGBA8_SRGB;
            }

            if (!pixels)
            {
                for (void *p : facesData)
                    stbi_image_free(p);
                FailedToLoadTexture(path);
                return nullptr;
            }
            facesData.push_back(pixels);
        }

        size_t pixelSize = Texture::GetPixelSize(format);
        size_t faceSize = static_cast<size_t>(texWidth) * texHeight * pixelSize;
        size_t totalSize = faceSize * 6;

        std::vector<uint8_t> fullData(totalSize);
        for (int i = 0; i < 6; i++)
        {
            std::memcpy(fullData.data() + (i * faceSize), facesData[i], faceSize);
            stbi_image_free(facesData[i]);
        }

        TextureDesc desc{
            .pixels = fullData.data(),
            .width = static_cast<uint32_t>(texWidth),
            .height = static_cast<uint32_t>(texHeight),
            .format = format,
            .generateMips = false,
            .isCube = true,
        };

        VE_CORE_INFO("CubeMap{} loaded", isHDR ? "(HDR)" : "");

        return std::make_shared<Texture>(desc);
    }

    std::shared_ptr<Texture> TextureLoader::LoadCubeMapPanorama(const std::string &panoramaPath)
    {
        int panWidth, panHeight, panChannels;
        bool isHDR = stbi_is_hdr(panoramaPath.c_str());
        void *panPixels = nullptr;

        if (isHDR)
            panPixels = stbi_loadf(panoramaPath.c_str(), &panWidth, &panHeight, &panChannels, STBI_rgb_alpha);
        else
            panPixels = stbi_load(panoramaPath.c_str(), &panWidth, &panHeight, &panChannels, STBI_rgb_alpha);

        if (!panPixels)
        {
            FailedToLoadTexture(panoramaPath);
            return nullptr;
        }

        int faceSize = panHeight / 2;
        TextureFormat format = isHDR ? TextureFormat::RGBA32_SFLOAT : TextureFormat::RGBA8_SRGB;
        size_t pixelSize = Texture::GetPixelSize(format);
        size_t faceByteSize = static_cast<size_t>(faceSize) * faceSize * pixelSize;

        std::vector<uint8_t> fullData(faceByteSize * 6);

        for (int face = 0; face < 6; ++face)
        {
            for (int y = 0; y < faceSize; ++y)
            {
                for (int x = 0; x < faceSize; ++x)
                {
                    float u = (static_cast<float>(x) + 0.5f) / faceSize;
                    float v = (static_cast<float>(y) + 0.5f) / faceSize;

                    glm::vec3 dir = GetCubeVector(face, u, v);

                    float phi = std::atan2(dir.z, dir.x);
                    float theta = std::acos(dir.y);

                    float panU = 1.0f - (phi + std::numbers::pi) / (2.0f * std::numbers::pi);
                    float panV = theta / std::numbers::pi;

                    int px = std::clamp(static_cast<int>(panU * panWidth), 0, panWidth - 1);
                    int py = std::clamp(static_cast<int>(panV * panHeight), 0, panHeight - 1);

                    size_t destIdx = (face * faceByteSize) + ((y * faceSize + x) * pixelSize);
                    size_t srcIdx = (py * panWidth + px) * pixelSize;

                    if (isHDR)
                    {
                        float *src = reinterpret_cast<float *>(panPixels) + (py * panWidth + px) * 4;
                        float *dest = reinterpret_cast<float *>(fullData.data() + destIdx);
                        std::memcpy(dest, src, sizeof(float) * 4);
                    }
                    else
                    {
                        uint8_t *src = reinterpret_cast<uint8_t *>(panPixels) + (py * panWidth + px) * 4;
                        uint8_t *dest = fullData.data() + destIdx;
                        std::memcpy(dest, src, sizeof(uint8_t) * 4);
                    }
                }
            }
        }

        stbi_image_free(panPixels);

        TextureDesc desc{
            .pixels = fullData.data(),
            .width = static_cast<uint32_t>(faceSize),
            .height = static_cast<uint32_t>(faceSize),
            .format = format,
            .generateMips = false,
            .isCube = true,
        };

        VE_CORE_INFO("CubeMap from panorama {} loaded to CPU", isHDR ? "(HDR)" : "");
        return std::make_shared<Texture>(desc);
    }

} // namespace ve
