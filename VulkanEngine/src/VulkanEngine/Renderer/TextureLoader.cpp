#include "TextureLoader.hpp"

#include <stb_image.h>

namespace ve
{
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
        bool isHDR = stbi_is_hdr(path.c_str());

        TextureFormat format;
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

        if (pixels)
        {
            stbi_image_free(pixels);
        }

        TextureDesc desc{
            .pixels = pixels,
            .width = static_cast<uint32_t>(texWidth),
            .height = static_cast<uint32_t>(texHeight),
            .format = format,
        };
        
        return std::make_shared<Texture>(desc);
    }

} // namespace ve
