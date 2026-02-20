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

    public:
        ~Texture() = default;

        int Width() const { return m_Width; }
        int Height() const { return m_Height; }
        int Channels() const { return m_Channels; }
        const unsigned char *Pixels() const { return m_Pixels.get(); }

    private:
        Texture(int width, int height, int channels, unsigned char *pixels);

    private:
        int m_Width, m_Height;
        int m_Channels;
        std::unique_ptr<unsigned char, void (*)(void *)> m_Pixels;
    };

}
