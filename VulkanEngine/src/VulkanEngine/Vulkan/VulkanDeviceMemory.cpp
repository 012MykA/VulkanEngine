#include "VulkanDeviceMemory.hpp"
#include "VulkanEngine/Core/Log.hpp"
#include "Debug/VulkanValidation.hpp"

#include <stdexcept>

namespace ve
{
    VulkanDeviceMemory::VulkanDeviceMemory(
        VkDevice device,
        VkPhysicalDevice physicalDevice,
        const size_t size,
        const uint32_t memoryTypeBits,
        const VkMemoryAllocateFlags allocateFLags,
        const VkMemoryPropertyFlags propertyFlags,
        const std::string &debugName) : m_Device(device), m_PhysicalDevice(physicalDevice), m_Size(size), m_DebugName(debugName)
    {
        VkMemoryAllocateFlagsInfo flagsInfo = {};
        flagsInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO;
        flagsInfo.pNext = nullptr;
        flagsInfo.flags = allocateFLags;

        VkMemoryAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.pNext = &flagsInfo;
        allocInfo.allocationSize = m_Size;
        allocInfo.memoryTypeIndex = FindMemoryType(memoryTypeBits, propertyFlags);

        VkResult result = vkAllocateMemory(m_Device, &allocInfo, nullptr, &m_Memory);
        CHECK_VK_RESULT(result);
        VE_CORE_TRACE("VulkanDeviceMemory ({0}) allocated (size: {1} bytes)", m_DebugName, m_Size);
    }

    VulkanDeviceMemory::VulkanDeviceMemory(VulkanDeviceMemory &&other) noexcept
        : m_Device(other.m_Device),
          m_PhysicalDevice(other.m_PhysicalDevice),
          m_Memory(other.m_Memory),
          m_Size(other.m_Size),
          m_DebugName(std::move(other.m_DebugName))
    {
        other.m_Memory = VK_NULL_HANDLE;
    }

    VulkanDeviceMemory::~VulkanDeviceMemory()
    {
        if (m_Memory != VK_NULL_HANDLE)
        {
            vkFreeMemory(m_Device, m_Memory, nullptr);
            VE_CORE_TRACE("VulkanDeviceMemory ({0}) freed", m_DebugName);
        }
    }

    void *VulkanDeviceMemory::Map(VkDeviceSize size, VkDeviceSize offset)
    {
        void *data;
        VkResult result = vkMapMemory(m_Device, m_Memory, offset, size, 0, &data);
        CHECK_VK_RESULT(result);

        return data;
    }

    void VulkanDeviceMemory::Unmap()
    {
        vkUnmapMemory(m_Device, m_Memory);
    }

    uint32_t VulkanDeviceMemory::FindMemoryType(const uint32_t typeFilter, const VkMemoryPropertyFlags propertyFlags) const
    {
        VkPhysicalDeviceMemoryProperties memProperties;
        vkGetPhysicalDeviceMemoryProperties(m_PhysicalDevice, &memProperties);

        for (uint32_t i = 0; i != memProperties.memoryTypeCount; ++i)
        {
            if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & propertyFlags) == propertyFlags)
            {
                return i;
            }
        }

        throw std::runtime_error("failed to find suitable memory type");
    }

} // namespace ve
