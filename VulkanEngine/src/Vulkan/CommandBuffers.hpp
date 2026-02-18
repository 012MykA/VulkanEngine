#pragma once

#include <vulkan/vulkan.h>

#include <cstdint>
#include <vector>

namespace VE
{
    class CommandPool;

    class CommandBuffers final
    {
    public:
        CommandBuffers(const CommandPool &commandPool, const uint32_t count);
        ~CommandBuffers();

        uint32_t Size() { return static_cast<uint32_t>(m_CommandBuffers.size()); }
        VkCommandBuffer &operator[](const size_t i) { return m_CommandBuffers[i]; }

        VkCommandBuffer Begin(size_t i);
        void End(size_t i);

    private:
        const CommandPool &m_CommandPool;

        std::vector<VkCommandBuffer> m_CommandBuffers;
    };

}
