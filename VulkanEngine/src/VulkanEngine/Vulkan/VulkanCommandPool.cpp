#include "VulkanCommandPool.hpp"
#include "Debug/VulkanValidation.hpp"
#include "VulkanEngine/Core/Log.hpp"

namespace ve
{
    VulkanCommandPool::VulkanCommandPool(VkDevice device, uint32_t queueFamilyIndex)
        : m_Device(device)
    {
        VkCommandPoolCreateInfo createInfo{
            .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            .pNext = nullptr,
            .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
            .queueFamilyIndex = queueFamilyIndex,
        };

        VkResult result = vkCreateCommandPool(m_Device, &createInfo, nullptr, &m_CommandPool);
        CHECK_VK_RESULT(result);
        VE_CORE_TRACE("VulkanCommandPool created");
    }

    VulkanCommandPool::~VulkanCommandPool()
    {
        vkDestroyCommandPool(m_Device, m_CommandPool, nullptr);
        VE_CORE_TRACE("VulkanCommandPool destroyed");
    }

} // namespace ve
