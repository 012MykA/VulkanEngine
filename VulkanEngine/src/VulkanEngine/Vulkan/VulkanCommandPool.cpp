#include "VulkanCommandPool.hpp"
#include "Debug/VulkanValidation.hpp"
#include "VulkanEngine/Core/Log.hpp"

namespace ve
{
    VulkanCommandPool::VulkanCommandPool(VkDevice device, uint32_t queueFamilyIndex, const std::string &debugName)
        : m_Device(device), m_DebugName(debugName)
    {
        VkCommandPoolCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        createInfo.queueFamilyIndex = queueFamilyIndex;
        createInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        createInfo.pNext = nullptr;

        VkResult result = vkCreateCommandPool(m_Device, &createInfo, nullptr, &m_CommandPool);
        CHECK_VK_RESULT(result);
        VE_CORE_TRACE("VulkanCommandPool ({0}) created", m_DebugName);
    }

    VulkanCommandPool::~VulkanCommandPool()
    {
        vkDestroyCommandPool(m_Device, m_CommandPool, nullptr);
        VE_CORE_TRACE("VulkanCommandPool ({0}) destroyed", m_DebugName);
    }

} // namespace ve
