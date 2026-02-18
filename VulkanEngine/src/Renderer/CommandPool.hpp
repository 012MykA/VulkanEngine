#pragma once

#include <vulkan/vulkan.h>

namespace VE
{
    class Device;

    class CommandPool final
    {
    public:
        explicit CommandPool(const Device &device);
        ~CommandPool();

        VkCommandPool Handle() const { return m_CommandPool; }
        const Device& GetDevice() const { return m_Device; }

    private:
        const Device &m_Device;

        VkCommandPool m_CommandPool = VK_NULL_HANDLE;
    };

}
