#include "VulkanBuffer.hpp"
#include "VulkanEngine/Core/Log.hpp"
#include "Debug/VulkanValidation.hpp"

#include <cassert>
#include <stdexcept>
#include <cstring>

namespace ve
{
    VulkanBuffer::VulkanBuffer(
        VkDevice device, VkPhysicalDevice physicalDevice,
        VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties)
        : m_Device(device), m_PhysicalDevice(physicalDevice), m_Size(size)
    {
        // Create buffer
        VkBufferCreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = m_Size;
        bufferInfo.usage = usage;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        VkResult result = vkCreateBuffer(m_Device, &bufferInfo, nullptr, &m_Buffer);
        CHECK_VK_RESULT(result);

        // Memory requirements
        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(m_Device, m_Buffer, &memRequirements);

        // Allocate memory
        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, properties);

        result = vkAllocateMemory(m_Device, &allocInfo, nullptr, &m_BufferMemory);
        CHECK_VK_RESULT(result);

        // Bind buffer to memory
        result = vkBindBufferMemory(m_Device, m_Buffer, m_BufferMemory, 0);
        CHECK_VK_RESULT(result);

        VE_CORE_TRACE("VulkanBuffer created (size: {0} bytes)", m_Size);
    }

    VulkanBuffer::~VulkanBuffer()
    {
        vkFreeMemory(m_Device, m_BufferMemory, nullptr);
        vkDestroyBuffer(m_Device, m_Buffer, nullptr);

        VE_CORE_TRACE("VulkanBuffer destroyed");
    }

    void VulkanBuffer::Map(VkDeviceSize size, VkDeviceSize offset)
    {
        assert(m_BufferMemory && "Cannot map buffer before memory allocation");

        VkResult result = vkMapMemory(m_Device, m_BufferMemory, offset, size, 0, &m_MappedData);
        CHECK_VK_RESULT(result);
    }

    void VulkanBuffer::Unmap()
    {
        assert(m_MappedData != nullptr && "Nothing to unmap");

        vkUnmapMemory(m_Device, m_BufferMemory);
        m_MappedData = nullptr;
    }

    void VulkanBuffer::WriteToBuffer(const void *data, VkDeviceSize size, VkDeviceSize offset)
    {
        assert(m_MappedData && "Cannot copy to unmapped buffer");

        if (size == VK_WHOLE_SIZE)
            size = m_Size;

        assert(offset + size <= m_Size);

        char *memOffset = static_cast<char *>(m_MappedData) + offset;
        std::memcpy(memOffset, data, size);
    }

    uint32_t VulkanBuffer::FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
    {
        VkPhysicalDeviceMemoryProperties memProperties;
        vkGetPhysicalDeviceMemoryProperties(m_PhysicalDevice, &memProperties);

        for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
        {
            if ((typeFilter & (1 << i)) &&
                (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
            {
                return i;
            }
        }

        throw std::runtime_error("failed to find suitable memory type!");
    }

} // namespace ve
