#pragma once

#include <vulkan/vulkan.h>

namespace VE
{
    class Device;
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

    private:
        const Device &m_Device;
        VkBuffer m_Buffer = VK_NULL_HANDLE;
    };

}
