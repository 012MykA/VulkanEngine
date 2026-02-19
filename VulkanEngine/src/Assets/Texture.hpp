#pragma once

#include <cstdint>
#include <string>
#include <memory>

namespace VE
{
    class Texture
    {
    public:
        static Texture LoadTexture(const std::string &filename);

    private:
        ~Texture() = default;

        const unsigned char *GetPixels() const { return m_Pixels.get(); }
        int GetWidth() const { return m_Width; }
        int GetHeight() const { return m_Height; }

    private:
        Texture(int width, int height, int channels, unsigned char *pixels);

    private:
        int m_Width, m_Height;
        int m_Channels;
        std::unique_ptr<unsigned char, void (*)(void *)> m_Pixels;
    };

}
