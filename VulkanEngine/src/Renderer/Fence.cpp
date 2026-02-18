#include "Fence.hpp"
#include "Device.hpp"
#include "Validation.hpp"

namespace VE
{
    Fence::Fence(const Device &device, bool signaled) : m_Device(device)
    {
        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = signaled ? VK_FENCE_CREATE_SIGNALED_BIT : 0;

        CheckVk(vkCreateFence(m_Device.Handle(), &fenceInfo, nullptr, &m_Fence), "create fence!");
    }

    Fence::Fence(Fence &&other) noexcept : m_Device(other.m_Device), m_Fence(other.m_Fence)
    {
        other.m_Fence = VK_NULL_HANDLE;
    }

    Fence::~Fence()
    {
        vkDestroyFence(m_Device.Handle(), m_Fence, nullptr);
    }

    void Fence::Reset()
    {
        CheckVk(vkResetFences(m_Device.Handle(), 1, &m_Fence), "reset fence!");
    }

    void Fence::Wait(uint64_t timeout)
    {
        CheckVk(vkWaitForFences(m_Device.Handle(), 1, &m_Fence, VK_TRUE, timeout), "wait for fence!");
    }

}
