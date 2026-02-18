#include "DeviceMemory.hpp"
#include "Device.hpp"
#include "Validation.hpp"

#include <stdexcept>

namespace VE
{
    DeviceMemory::DeviceMemory(
        const Device &device,
        const size_t size,
        const uint32_t memoryTypeBits,
        const VkMemoryAllocateFlags allocateFLags,
        const VkMemoryPropertyFlags propertyFlags) : m_Device(device)
    {
        VkMemoryAllocateFlagsInfo flagsInfo = {};
        flagsInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO;
        flagsInfo.pNext = nullptr;
        flagsInfo.flags = allocateFLags;

        VkMemoryAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.pNext = &flagsInfo;
        allocInfo.allocationSize = size;
        allocInfo.memoryTypeIndex = FindMemoryType(memoryTypeBits, propertyFlags);

        CheckVk(vkAllocateMemory(m_Device.Handle(), &allocInfo, nullptr, &m_Memory), "allocate memory!");
    }

    DeviceMemory::DeviceMemory(DeviceMemory &&other) noexcept : m_Device(other.m_Device), m_Memory(other.m_Memory)
    {
        other.m_Memory = VK_NULL_HANDLE;
    }

    DeviceMemory::~DeviceMemory()
    {
        vkFreeMemory(m_Device.Handle(), m_Memory, nullptr);
    }

    void *DeviceMemory::Map(size_t offset, size_t size)
    {
        void *data;
        CheckVk(vkMapMemory(m_Device.Handle(), m_Memory, offset, size, 0, &data), "map memory!");

        return data;
    }

    void DeviceMemory::Unmap()
    {
        vkUnmapMemory(m_Device.Handle(), m_Memory);
    }

    uint32_t DeviceMemory::FindMemoryType(const uint32_t typeFilter, const VkMemoryPropertyFlags propertyFlags) const
    {
        VkPhysicalDeviceMemoryProperties memProperties;
        vkGetPhysicalDeviceMemoryProperties(m_Device.GetPhysicalDevice(), &memProperties);

        for (uint32_t i = 0; i != memProperties.memoryTypeCount; ++i)
        {
            if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & propertyFlags) == propertyFlags)
            {
                return i;
            }
        }

        throw std::runtime_error("failed to find suitable memory type");
    }

}
