#pragma once

#include <vulkan/vulkan.h>

#include <string>

namespace ve
{
    class VulkanDeviceMemory final
    {
    public:
        VulkanDeviceMemory(const VulkanDeviceMemory &) = delete;
        VulkanDeviceMemory &operator=(const VulkanDeviceMemory &) = delete;
        VulkanDeviceMemory &operator=(VulkanDeviceMemory &&) = delete;

        VulkanDeviceMemory(VkDevice device,
                           VkPhysicalDevice physicalDevice,
                           const size_t size,
                           const uint32_t memoryTypeBits,
                           const VkMemoryAllocateFlags allocateFLags,
                           const VkMemoryPropertyFlags propertyFlags,
                           const std::string &debugName = "Unnamed");
        VulkanDeviceMemory(VulkanDeviceMemory &&other) noexcept;
        ~VulkanDeviceMemory();

        VkDeviceMemory Handle() const { return m_Memory; }

        void *Map(VkDeviceSize size, VkDeviceSize offset = 0);
        void Unmap();

        uint32_t FindMemoryType(const uint32_t typeFilter, const VkMemoryPropertyFlags propertyFlags) const;

    private:
        VkDevice m_Device;
        VkPhysicalDevice m_PhysicalDevice;
        VkDeviceMemory m_Memory = VK_NULL_HANDLE;
        VkDeviceSize m_Size;

        std::string m_DebugName;
    };

} // namespace ve
