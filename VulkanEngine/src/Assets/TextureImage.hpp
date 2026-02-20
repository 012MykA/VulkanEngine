#pragma once

#include <vulkan/vulkan.h>

#include <memory>

namespace VE
{
    class CommandPool;
    class Texture;
    class Image;
    class DeviceMemory;
    class ImageView;
    class Sampler;

    class TextureImage final
    {
    public:
        TextureImage(const CommandPool &commandPool, const Texture &texture);
        ~TextureImage();

        const ImageView &GetImageView() const { return *m_ImageView; }
        const Sampler &GetSampler() const { return *m_Sampler; }

    private:
        std::unique_ptr<Image> m_Image;
        std::unique_ptr<DeviceMemory> m_ImageMemory;
        std::unique_ptr<ImageView> m_ImageView;
        std::unique_ptr<Sampler> m_Sampler;
    };

}
