#include "TextureLoader.hpp"
#include "VulkanEngine/Core/Log.hpp"

#include <stb_image.h>

#include <cstring>
#include <vector>
#include <filesystem>

namespace ve
{
    namespace
    {
        void FailedToLoadTexture(const std::string &path)
        {
            VE_CORE_ERROR("Failed to load texture '{}'", path);
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

} // namespace ve
