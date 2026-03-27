#include "VulkanBuffer.hpp"

#include <cassert>

namespace ve
{
    VulkanBuffer::VulkanBuffer(const VulkanAllocator &allocator, const BufferDesc &desc)
        : m_Allocator(&allocator), m_Desc(desc)
    {
        VkBufferUsageFlags usage = static_cast<VkBufferUsageFlags>(desc.type) | desc.extraUsage;

        VkBufferCreateInfo bufferInfo{
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .size = desc.size,
            .usage = usage,
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        };

        m_Allocation = allocator.AllocateBuffer(bufferInfo, desc.allocation, m_Buffer);
    }

    VulkanBuffer::~VulkanBuffer()
    {
        Destroy();
    }

    VulkanBuffer::VulkanBuffer(VulkanBuffer &&other) noexcept
        : m_Allocator(other.m_Allocator), m_Buffer(other.m_Buffer),
          m_Allocation(other.m_Allocation), m_Desc(other.m_Desc)
    {
        other.m_Buffer = VK_NULL_HANDLE;
        other.m_Allocation.handle = VK_NULL_HANDLE;
        other.m_Allocator = nullptr;
    }

    VulkanBuffer &VulkanBuffer::operator=(VulkanBuffer &&other) noexcept
    {
        if (this != &other)
        {
            Destroy();

            m_Allocator = other.m_Allocator;
            m_Buffer = other.m_Buffer;
            m_Allocation = other.m_Allocation;
            m_Desc = other.m_Desc;

            other.m_Buffer = VK_NULL_HANDLE;
            other.m_Allocation.handle = VK_NULL_HANDLE;
            other.m_Allocator = nullptr;
        }
        return *this;
    }

    void VulkanBuffer::Upload(const void *data, VkDeviceSize size, VkDeviceSize offset) const
    {
        assert(size + offset <= m_Desc.size, "VulkanBuffer::Upload - out of bounds");

        if (m_Allocation.mappedPtr)
        {
            std::memcpy(static_cast<uint8_t *>(m_Allocation.mappedPtr) + offset, data, size);
            m_Allocator->FlushAllocation(m_Allocation, offset, size);
            m_Allocator->UnmapMemory(m_Allocation);
        }
    }

    void VulkanBuffer::CopyTo(VkCommandBuffer cmd, const VulkanBuffer &dst,
                              VkDeviceSize srcOffset,
                              VkDeviceSize dstOffset,
                              VkDeviceSize size) const
    {
        VkDeviceSize copySize = (size == VK_WHOLE_SIZE) ? m_Desc.size : size;

        VkBufferCopy region{
            .srcOffset = srcOffset,
            .dstOffset = dstOffset,
            .size = copySize,
        };

        vkCmdCopyBuffer(cmd, m_Buffer, dst.m_Buffer, 1, &region);
    }

    void VulkanBuffer::Destroy()
    {
        if (m_Buffer != VK_NULL_HANDLE && m_Allocator)
        {
            m_Allocator->FreeBuffer(m_Buffer, m_Allocation);
            m_Buffer = VK_NULL_HANDLE;
            m_Allocation.handle = VK_NULL_HANDLE;
            m_Allocator = nullptr;
        }
    }

} // namespace ve
