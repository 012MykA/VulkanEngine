#pragma once

#include "VulkanDeviceMemory.hpp"

#include <vulkan/vulkan.h>

#include <string>

namespace ve
{
    class VulkanBuffer
    {
    public:
        VulkanBuffer(VkDevice device, VkDeviceSize size, VkBufferUsageFlags usage, const std::string &debugName = "Unnamed");
        ~VulkanBuffer();

        VulkanBuffer(const VulkanBuffer &) = delete;
        VulkanBuffer &operator=(const VulkanBuffer &) = delete;

    public:
        VulkanDeviceMemory AllocateMemory(VkPhysicalDevice physicalDevice, const VkMemoryPropertyFlags propertyFlags);
        VulkanDeviceMemory AllocateMemory(VkPhysicalDevice physicalDevice, const VkMemoryAllocateFlags allocateFlags, const VkMemoryPropertyFlags propertyFlags);

        void CopyFrom(VkBuffer srcBuffer, VkDeviceSize size, VkCommandPool commandPool, VkQueue graphicsQueue);

    public:
        // Getters
        VkBuffer GetBuffer() const { return m_Buffer; }
        VkMemoryRequirements GetMemoryRequirements() const;
        VkDeviceAddress GetDeviceAddress() const;

    private:
        VkDevice m_Device;
        VkBuffer m_Buffer = VK_NULL_HANDLE;

        std::string m_DebugName;
    };

} // namespace ve
