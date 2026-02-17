#include "CommandBuffers.hpp"
#include "Device.hpp"
#include "CommandPool.hpp"
#include "Validation.hpp"

namespace VE
{
    CommandBuffers::CommandBuffers(const Device &device, const CommandPool &commandPool, uint32_t count)
        : m_Device(device), m_CommandPool(commandPool)
    {
        m_CommandBuffers.resize(count);

        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = m_CommandPool.Handle();
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = count;

        CheckVk(vkAllocateCommandBuffers(m_Device.Handle(), &allocInfo, m_CommandBuffers.data()), "allocate command buffers!");
    }

    CommandBuffers::~CommandBuffers()
    {
        vkFreeCommandBuffers(m_Device.Handle(),
                             m_CommandPool.Handle(),
                             static_cast<uint32_t>(m_CommandBuffers.size()),
                             m_CommandBuffers.data());
    }

}
