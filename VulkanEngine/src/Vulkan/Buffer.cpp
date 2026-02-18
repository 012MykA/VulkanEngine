#include "Buffer.hpp"
#include "Device.hpp"
#include "DeviceMemory.hpp"
#include "CommandBuffers.hpp"
#include "Validation.hpp"

namespace VE
{
    Buffer::Buffer(const Device &device, size_t size, VkBufferUsageFlags usage) : m_Device(device)
    {
        VkBufferCreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = size;
        bufferInfo.usage = usage;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        CheckVk(vkCreateBuffer(device.Handle(), &bufferInfo, nullptr, &m_Buffer), "create buffer!");
    }

    Buffer::~Buffer()
    {
        vkDestroyBuffer(m_Device.Handle(), m_Buffer, nullptr);
    }

    DeviceMemory Buffer::AllocateMemory(const VkMemoryPropertyFlags propertyFlags)
    {
        return AllocateMemory(0, propertyFlags);
    }

    DeviceMemory Buffer::AllocateMemory(const VkMemoryAllocateFlags allocateFlags, const VkMemoryPropertyFlags propertyFlags)
    {
        const auto requirements = GetMemoryRequirements();
        DeviceMemory memory(m_Device, requirements.size, requirements.memoryTypeBits, allocateFlags, propertyFlags);

        CheckVk(vkBindBufferMemory(m_Device.Handle(), m_Buffer, memory.Handle(), 0), "bind buffer memory!");

        return memory;
    }

    VkMemoryRequirements Buffer::GetMemoryRequirements() const
    {
        VkMemoryRequirements requirements;
        vkGetBufferMemoryRequirements(m_Device.Handle(), m_Buffer, &requirements);
        return requirements;
    }

    VkDeviceAddress Buffer::GetDeviceAddress() const
    {
        VkBufferDeviceAddressInfo info{};
        info.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
        info.pNext = nullptr;
        info.buffer = m_Buffer;

        return vkGetBufferDeviceAddress(m_Device.Handle(), &info);
    }

    void Buffer::CopyFrom(const CommandPool &commandPool, const Buffer &src, VkDeviceSize size)
    {
        CommandBuffers commandBuffers(commandPool, 1);

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        vkBeginCommandBuffer(commandBuffers[0], &beginInfo);

        VkBufferCopy copyRegion{};
        copyRegion.srcOffset = 0; // Optional
        copyRegion.dstOffset = 0; // Optional
        copyRegion.size = size;

        vkCmdCopyBuffer(commandBuffers[0], src.Handle(), m_Buffer, 1, &copyRegion);

        vkEndCommandBuffer(commandBuffers[0]);

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffers[0];

        const auto graphicsQueue = commandPool.GetDevice().GraphicsQueue();
        vkQueueSubmit(graphicsQueue, 1, &submitInfo, nullptr);
        vkQueueWaitIdle(graphicsQueue);
    }

}
