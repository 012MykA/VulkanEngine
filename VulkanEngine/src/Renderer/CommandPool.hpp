#pragma once

#include <vulkan/vulkan.h>

namespace VE
{
    class Device;

    class CommandPool final
    {
    public:
        CommandPool(const Device &device);
        ~CommandPool();

        VkCommandPool Handle() const { return m_CommandPool; }

    private:
        const Device &m_Device;

        VkCommandPool m_CommandPool = VK_NULL_HANDLE;
    };

}
