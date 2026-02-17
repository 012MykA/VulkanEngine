#pragma once

#include <vulkan/vulkan.h>

#include <cstdint>
#include <vector>

namespace VE
{
    class Device;
    class CommandPool;

    class CommandBuffers final
    {
    public:
        CommandBuffers(const Device &device, const CommandPool &commandPool, uint32_t count);
        ~CommandBuffers();

        const std::vector<VkCommandBuffer> &Handle() const { return m_CommandBuffers; }

    private:
        const Device &m_Device;
        const CommandPool &m_CommandPool;

        std::vector<VkCommandBuffer> m_CommandBuffers;
    };

}
