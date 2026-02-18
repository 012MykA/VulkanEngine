#pragma once

#include <vulkan/vulkan.h>

#include <cstdint>

namespace VE
{
    class Device;

    class Fence final
    {
    public:
        Fence(const Fence &) = delete;
        Fence &operator=(const Fence &) = delete;
        Fence &operator=(Fence &&) = delete;

        explicit Fence(const Device &device, bool signaled);
        Fence(Fence &&other) noexcept;
        ~Fence();

        const VkFence &Handle() const { return m_Fence; }

        void Reset();
        void Wait(uint64_t timeout);

    private:
        const Device &m_Device;
        VkFence m_Fence = VK_NULL_HANDLE;
    };

}
