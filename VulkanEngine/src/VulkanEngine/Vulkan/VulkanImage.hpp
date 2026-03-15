#pragma once

#include "VulkanDeviceMemory.hpp"

#include <vulkan/vulkan.h>

#include <string>

namespace ve
{
    class VulkanImage
    {
    public:
        VulkanImage(VkDevice device, const VkExtent2D extent, const VkFormat format,
                    VkImageTiling tiling, VkImageUsageFlags usage, const std::string &debugName = "Unnamed");
        ~VulkanImage();

        VulkanDeviceMemory AllocateMemory(VkPhysicalDevice physicalDevice, const VkMemoryPropertyFlags properties) const;
        void CopyFrom(VkBuffer srcBuffer, VkCommandPool commandPool, VkQueue graphicsQueue);
        void TransitionImageLayout(VkImageLayout newLayout, VkCommandPool commandPool, VkQueue graphicsQueue);

    public:
        // Getters
        VkImage GetImage() const { return m_Image; }
        VkFormat GetFormat() const { return m_Format; }
        uint32_t GetMipLevels() const { return m_MipLevels; }
        VkMemoryRequirements GetMemoryRequirements() const;

    private:
        VkDevice m_Device;
        VkImage m_Image = VK_NULL_HANDLE;

        VkImageLayout m_ImageLayout;
        const VkExtent2D m_Extent;
        const VkFormat m_Format;
        uint32_t m_MipLevels = 1;

        std::string m_DebugName;
    };

} // namespace ve
