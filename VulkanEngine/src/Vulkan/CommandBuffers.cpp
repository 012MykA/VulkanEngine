#include "CommandBuffers.hpp"
#include "Device.hpp"
#include "CommandPool.hpp"
#include "Validation.hpp"

namespace VE
{
    CommandBuffers::CommandBuffers(const CommandPool &commandPool, const uint32_t count)
        : m_CommandPool(commandPool)
    {
        m_CommandBuffers.resize(count);

        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = m_CommandPool.Handle();
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = count;

        CheckVk(vkAllocateCommandBuffers(m_CommandPool.GetDevice().Handle(), &allocInfo, m_CommandBuffers.data()), "allocate command buffers!");
    }

    CommandBuffers::~CommandBuffers()
    {
        vkFreeCommandBuffers(m_CommandPool.GetDevice().Handle(),
                             m_CommandPool.Handle(),
                             static_cast<uint32_t>(m_CommandBuffers.size()),
                             m_CommandBuffers.data());
    }

    VkCommandBuffer CommandBuffers::Begin(size_t i)
    {
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
        beginInfo.pInheritanceInfo = nullptr; // Optional

        CheckVk(vkBeginCommandBuffer(m_CommandBuffers[i], &beginInfo), "begin command buffer!");

        return m_CommandBuffers[i];
    }

    void CommandBuffers::End(size_t i)
    {
        CheckVk(vkEndCommandBuffer(m_CommandBuffers[i]), "record command buffer!");
    }

}
