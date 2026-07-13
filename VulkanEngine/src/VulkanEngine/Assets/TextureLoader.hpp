#pragma once

#include "Texture.hpp"

#include <string>
#include <memory>
#include <cstdint>
#include <array>

namespace ve
{
    class TextureLoader
    {
    public:
        static std::shared_ptr<Texture> CreateSolid(uint8_t r, uint8_t g, uint8_t b, uint8_t a);

        // If the image is HDR it sets format to RGBA32_SFLOAT, else RGBA8_SRGB
        static std::shared_ptr<Texture> Load(const std::string &path);

        // If the image is HDR it sets format to RGBA32_SFLOAT, else RGBA8_SRGB
        static std::shared_ptr<Texture> LoadCubeMap(const std::array<std::string, 6> &paths);

        static std::shared_ptr<Texture> LoadCubeMapPanorama(const std::string &panoramaPath);
    };

} // namespace ve
