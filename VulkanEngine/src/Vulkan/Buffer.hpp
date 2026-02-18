#pragma once

#include "CommandPool.hpp"

#include <vulkan/vulkan.h>

#include <vector>
#include <utility>
#include <memory>

namespace VE
{
    class Device;
    class CommandPool;
    class DeviceMemory;

    class Buffer final
    {
    public:
        Buffer(const Device &device, size_t size, VkBufferUsageFlags usage);
        ~Buffer();

        VkBuffer Handle() const { return m_Buffer; }

        DeviceMemory AllocateMemory(const VkMemoryPropertyFlags propertyFlags);
        DeviceMemory AllocateMemory(const VkMemoryAllocateFlags allocateFlags, const VkMemoryPropertyFlags propertyFlags);

        VkMemoryRequirements GetMemoryRequirements() const;
        VkDeviceAddress GetDeviceAddress() const;

        void CopyFrom(const CommandPool &commandPool, const Buffer &src, VkDeviceSize size);

        template <typename T>
        static std::pair<std::unique_ptr<Buffer>, std::unique_ptr<DeviceMemory>> CreateFromData(
            const CommandPool &commandPool,
            const std::vector<T> &data,
            VkBufferUsageFlags usageFlags);

    private:
        const Device &m_Device;
        VkBuffer m_Buffer = VK_NULL_HANDLE;
    };

    template <typename T>
    inline std::pair<std::unique_ptr<Buffer>, std::unique_ptr<DeviceMemory>> Buffer::CreateFromData(
        const CommandPool &commandPool,
        const std::vector<T> &data,
        VkBufferUsageFlags usageFlags)
    {
        const auto &device = commandPool.GetDevice();

        VkDeviceSize size = sizeof(T) * data.size();

        Buffer stagingBuffer(device, size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
        DeviceMemory stagingMemory = stagingBuffer.AllocateMemory(
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
            VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        void *mapped = stagingMemory.Map(0, size);
        std::memcpy(mapped, data.data(), size);
        stagingMemory.Unmap();

        auto buffer = std::make_unique<Buffer>(device, size, usageFlags | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
        auto memory = std::make_unique<DeviceMemory>(buffer->AllocateMemory(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT));

        buffer->CopyFrom(commandPool, stagingBuffer, size);

        return {std::move(buffer), std::move(memory)};
    }

}
