#pragma once

#include "VulkanImage.hpp"

#include <vulkan/vulkan.h>

#include <memory>

namespace ve
{
    class VulkanLogicalDevice;
    class VulkanAllocator;
    class VulkanPhysicalDevice;

    class VulkanDepthBuffer
    {
    public:
        VulkanDepthBuffer(
            const VulkanLogicalDevice &logicalDevice,
            const VulkanPhysicalDevice &physicalDevice,
            const VulkanAllocator &allocator,
            uint32_t width, uint32_t height);
        ~VulkanDepthBuffer();

        VulkanDepthBuffer(const VulkanDepthBuffer &) = delete;
        VulkanDepthBuffer &operator=(const VulkanDepthBuffer &) = delete;

        void Recreate(const VulkanAllocator &allocator, uint32_t width, uint32_t height);

    public: // Getters
        VkImageView GetView() const { return m_Image->GetView(); }
        VkFormat GetFormat() const { return m_Format; }

    private:
        void Create(const VulkanAllocator &allocator, uint32_t width, uint32_t height);

        static VkFormat FindDepthFormat(VkPhysicalDevice physicalDevice);

    private:
        const VulkanLogicalDevice *m_Device = nullptr;
        VkFormat m_Format = VK_FORMAT_UNDEFINED;
        std::unique_ptr<VulkanImage> m_Image;
    };

} // namespace ve
