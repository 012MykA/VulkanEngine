#include "Semaphore.hpp"
#include "Device.hpp"
#include "Validation.hpp"

namespace VE
{
    Semaphore::Semaphore(const Device &device) : m_Device(device)
    {
        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        CheckVk(vkCreateSemaphore(m_Device.Handle(), &semaphoreInfo, nullptr, &m_Semaphore), "create semaphore!");
    }

    Semaphore::Semaphore(Semaphore &&other) noexcept : m_Device(other.m_Device), m_Semaphore(other.m_Semaphore)
    {
        other.m_Semaphore = VK_NULL_HANDLE;
    }

    Semaphore::~Semaphore()
    {
        vkDestroySemaphore(m_Device.Handle(), m_Semaphore, nullptr);
    }

}
