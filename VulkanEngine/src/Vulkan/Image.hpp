#pragma once

#include "DeviceMemory.hpp"

#include <vulkan/vulkan.h>

namespace VE
{
    class Device;
    class CommandPool;
    class Buffer;

    class Image
    {
    public:
        Image(const Device &device, const VkExtent2D extent, const VkFormat format);
        Image(const Device &device, const VkExtent2D extent, const VkFormat format, VkImageTiling tiling, VkImageUsageFlags isage);
        ~Image();

        VkImage Handle() const { return m_Image; }

        VkFormat GetFormat() const { return m_Format; }

        DeviceMemory AllocateMemory(const VkMemoryPropertyFlags properties) const;
        VkMemoryRequirements GetMemoryRequirements() const;

        void TransitionImageLayout(const CommandPool &commandPool, VkImageLayout newLayout);
        void CopyFrom(const CommandPool &commandPool, const Buffer &buffer);

    private:
        const Device &m_Device;
        const VkExtent2D m_Extent;
        const VkFormat m_Format;

        VkImageLayout m_ImageLayout;
        VkImage m_Image = VK_NULL_HANDLE;
    };

}
