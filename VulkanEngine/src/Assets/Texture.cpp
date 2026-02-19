#include "Texture.hpp"

#include <stb_image.h>

#include <stdexcept>
#include <iostream>

namespace VE
{
    Texture Texture::LoadTexture(const std::string &filename)
    {
        int width, height, channels;
        unsigned char *pixels = stbi_load(filename.c_str(), &width, &height, &channels, STBI_rgb_alpha);
        if (!pixels)
            throw std::runtime_error("failed to load texture image!");

        return Texture(width, height, channels, pixels);
    }

    Texture::Texture(int width, int height, int channels, unsigned char *pixels)
        : m_Width(width), m_Height(height), m_Channels(channels), m_Pixels(pixels, stbi_image_free)
    {
    }

}
