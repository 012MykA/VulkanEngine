#pragma once

#include <vulkan/vulkan.h>

#include <string>

namespace ve
{
    class VulkanCommandPool
    {
    public:
        VulkanCommandPool(VkDevice device, uint32_t queueFamilyIndex, const std::string& debugName = "Unnamed");
        ~VulkanCommandPool();

        VkCommandPool GetCommandPool() const { return m_CommandPool; }

    private:
        VkDevice m_Device;
        VkCommandPool m_CommandPool = VK_NULL_HANDLE;

        std::string m_DebugName;
    };

} // namespace ve
