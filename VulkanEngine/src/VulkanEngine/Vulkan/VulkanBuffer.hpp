#pragma once

#include <vulkan/vulkan.h>

namespace ve
{
    class VulkanBuffer
    {
    public:
        VulkanBuffer(VkDevice device, VkPhysicalDevice physicalDevice, VkDeviceSize size,
                     VkBufferUsageFlags usage, VkMemoryPropertyFlags properties);
        ~VulkanBuffer();

        VulkanBuffer(const VulkanBuffer &) = delete;
        VulkanBuffer &operator=(const VulkanBuffer &) = delete;

        void Map(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
        void Unmap();
        void WriteToBuffer(const void *data, VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);

        VkBuffer GetBuffer() const { return m_Buffer; }

    private:
        uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

    private:
        VkDevice m_Device;
        VkPhysicalDevice m_PhysicalDevice;
        VkBuffer m_Buffer = VK_NULL_HANDLE;
        VkDeviceMemory m_BufferMemory = VK_NULL_HANDLE;
        VkDeviceSize m_Size;
        void *m_MappedData = nullptr;
    };

} // namespace ve
