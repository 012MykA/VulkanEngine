#pragma once

#include <vulkan/vulkan.h>

namespace VE
{
    class Device;

    class Semaphore final
    {
    public:
        Semaphore(const Semaphore &) = delete;
        Semaphore &operator=(const Semaphore &) = delete;
        Semaphore &operator=(Semaphore &&) = delete;

        explicit Semaphore(const Device &device);
        Semaphore(Semaphore &&other) noexcept;
        ~Semaphore();

        VkSemaphore Handle() const { return m_Semaphore; }
        const Device &GetDevice() const { return m_Device; }

    private:
        const Device &m_Device;
        VkSemaphore m_Semaphore = VK_NULL_HANDLE;
    };

}
