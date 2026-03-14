#include "VulkanBuffer.hpp"
#include "VulkanEngine/Core/Log.hpp"
#include "Debug/VulkanValidation.hpp"

namespace ve
{
    VulkanBuffer::VulkanBuffer(VkDevice device, VkDeviceSize size, VkBufferUsageFlags usage, const std::string &debugName)
        : m_Device(device), m_DebugName(debugName)
    {
        // Create buffer
        VkBufferCreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = size;
        bufferInfo.usage = usage;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        VkResult result = vkCreateBuffer(m_Device, &bufferInfo, nullptr, &m_Buffer);
        CHECK_VK_RESULT(result);

        VE_CORE_TRACE("VulkanBuffer ({0}) created (size: {1} bytes)", m_DebugName, size);
    }

    VulkanBuffer::~VulkanBuffer()
    {
        vkDestroyBuffer(m_Device, m_Buffer, nullptr);
        VE_CORE_TRACE("VulkanBuffer ({0}) destroyed", m_DebugName);
    }

    VulkanDeviceMemory VulkanBuffer::AllocateMemory(VkPhysicalDevice physicalDevice, const VkMemoryPropertyFlags propertyFlags)
    {
        return AllocateMemory(physicalDevice, 0, propertyFlags);
    }

    VulkanDeviceMemory VulkanBuffer::AllocateMemory(VkPhysicalDevice physicalDevice, const VkMemoryAllocateFlags allocateFlags, const VkMemoryPropertyFlags propertyFlags)
    {
        const auto requirements = GetMemoryRequirements();
        VulkanDeviceMemory memory(m_Device, physicalDevice, requirements.size, requirements.memoryTypeBits, allocateFlags, propertyFlags, m_DebugName);

        VkResult result = vkBindBufferMemory(m_Device, m_Buffer, memory.Handle(), 0);
        CHECK_VK_RESULT(result);

        return memory;
    }

    VkMemoryRequirements VulkanBuffer::GetMemoryRequirements() const
    {
        VkMemoryRequirements requirements;
        vkGetBufferMemoryRequirements(m_Device, m_Buffer, &requirements);
        return requirements;
    }

    VkDeviceAddress VulkanBuffer::GetDeviceAddress() const
    {
        VkBufferDeviceAddressInfo info{};
        info.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
        info.pNext = nullptr;
        info.buffer = m_Buffer;

        return vkGetBufferDeviceAddress(m_Device, &info);
    }

} // namespace ve
