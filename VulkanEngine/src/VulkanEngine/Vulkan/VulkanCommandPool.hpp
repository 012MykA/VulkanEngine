#pragma once

#include <vulkan/vulkan.h>

namespace ve
{
    class VulkanCommandPool
    {
    public:
        VulkanCommandPool(VkDevice device, uint32_t queueFamilyIndex);
        ~VulkanCommandPool();

    public:
        // Getters
        VkCommandPool GetCommandPool() const { return m_CommandPool; }
        VkDevice GetDevice() const { return m_Device; }

    private:
        VkDevice m_Device;
        VkCommandPool m_CommandPool = VK_NULL_HANDLE;
    };

} // namespace ve
