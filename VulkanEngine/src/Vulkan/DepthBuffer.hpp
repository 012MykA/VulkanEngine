#pragma once

#include <vulkan/vulkan.h>

#include <memory>

namespace VE
{
    class CommandPool;
    class Image;
    class DeviceMemory;
    class ImageView;

    class DepthBuffer final
    {
    public:
        DepthBuffer(const CommandPool &commandPool, const VkExtent2D extent);
        ~DepthBuffer();

        VkFormat GetFormat() const { return m_Format; }
        const ImageView &GetImageView() const { return *m_ImageView; }

        static bool HasStencilComponent(const VkFormat format)
        {
            return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
        }

    private:
        const VkFormat m_Format;
        std::unique_ptr<Image> m_Image;
        std::unique_ptr<DeviceMemory> m_ImageMemory;
        std::unique_ptr<ImageView> m_ImageView;
    };

}
