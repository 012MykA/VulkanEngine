#include "CommandPool.hpp"
#include "Device.hpp"
#include "Validation.hpp"

namespace VE
{
    CommandPool::CommandPool(const Device &device) : m_Device(device)
    {
        VkCommandPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        poolInfo.queueFamilyIndex = m_Device.Queues().GraphicsFamily.value();

        CheckVk(vkCreateCommandPool(m_Device.Handle(), &poolInfo, nullptr, &m_CommandPool), "create command pool!");
    }

    CommandPool::~CommandPool()
    {
        vkDestroyCommandPool(m_Device.Handle(), m_CommandPool, nullptr);
    }

}
