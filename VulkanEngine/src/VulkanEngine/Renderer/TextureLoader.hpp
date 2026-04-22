#pragma once

#include "Texture.hpp"

#include <string>
#include <memory>
#include <cstdint>

namespace ve
{
    class TextureLoader
    {
    public:
        TextureLoader() = default;
        ~TextureLoader() = default;

        std::shared_ptr<Texture> CreateSolid(uint8_t r, uint8_t g, uint8_t b, uint8_t a);

        std::shared_ptr<Texture> Load(const std::string &path);
    };

} // namespace ve
