#pragma once

#include <vulkan/vulkan.h>

namespace ve
{
    class VulkanCommandPool
    {
    public:
        VulkanCommandPool(VkDevice device, uint32_t queueFamilyIndex);
        ~VulkanCommandPool();

        VkCommandPool GetCommandPool() const { return m_CommandPool; }

    private:
        VkDevice m_Device;
        VkCommandPool m_CommandPool = VK_NULL_HANDLE;
    };

} // namespace ve
