#pragma once

#include <vulkan/vulkan.h>

namespace VE
{
    class Device;

    class DeviceMemory final
    {
    public:
        DeviceMemory(const DeviceMemory &) = delete;
        DeviceMemory &operator=(const DeviceMemory &) = delete;
        DeviceMemory &operator=(DeviceMemory &&) = delete;

        DeviceMemory(const Device &device,
                     const size_t size,
                     const uint32_t memoryTypeBits,
                     const VkMemoryAllocateFlags allocateFLags,
                     const VkMemoryPropertyFlags propertyFlags);
        DeviceMemory(DeviceMemory &&other) noexcept;
        ~DeviceMemory();

        VkDeviceMemory Handle() const { return m_Memory; }

        void *Map(size_t offset, size_t size);
        void Unmap();

        uint32_t FindMemoryType(const uint32_t typeFilter, const VkMemoryPropertyFlags propertyFlags) const;

    private:
        const Device &m_Device;

        VkDeviceMemory m_Memory = VK_NULL_HANDLE;
    };

}
