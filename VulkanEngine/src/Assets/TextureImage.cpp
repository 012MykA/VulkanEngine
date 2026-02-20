#include "TextureImage.hpp"
#include "CommandPool.hpp"
#include "Texture.hpp"
#include "Image.hpp"
#include "DeviceMemory.hpp"
#include "ImageView.hpp"
#include "Sampler.hpp"
#include "Buffer.hpp"

namespace VE
{
    TextureImage::TextureImage(const CommandPool &commandPool, const Texture &texture)
    {
        const VkDeviceSize imageSize = texture.Width() * texture.Height() * 4;
        const auto &device = commandPool.GetDevice();

        Buffer stagingBuffer(device, imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
        DeviceMemory stagingMemory = stagingBuffer.AllocateMemory(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                                                  VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        void *data = stagingMemory.Map(0, imageSize);
        std::memcpy(data, texture.Pixels(), imageSize);
        stagingMemory.Unmap();

        VkExtent2D imageExtent = {static_cast<uint32_t>(texture.Width()), static_cast<uint32_t>(texture.Height())};
        m_Image = std::make_unique<Image>(device, imageExtent, VK_FORMAT_R8G8B8A8_SRGB);
        m_ImageMemory = std::make_unique<DeviceMemory>(m_Image->AllocateMemory(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT));
        m_ImageView = std::make_unique<ImageView>(device, m_Image->Handle(), m_Image->GetFormat(), VK_IMAGE_ASPECT_COLOR_BIT);
        m_Sampler = std::make_unique<Sampler>(device, SamplerConfig());

        m_Image->TransitionImageLayout(commandPool, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
        m_Image->CopyFrom(commandPool, stagingBuffer);
        m_Image->TransitionImageLayout(commandPool, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    }

    TextureImage::~TextureImage() = default;

}
