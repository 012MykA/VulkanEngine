#pragma once

#include "VulkanSyncObjects.hpp"

#include <vulkan/vulkan.h>

#include <memory>
#include <cstdint>

namespace ve
{
    class VulkanLogicalDevice;
    class VulkanCommandPool;

    struct VulkanFrameData
    {
        VkCommandBuffer commandBuffer = VK_NULL_HANDLE;
        std::unique_ptr<VulkanSyncObjects> syncObjects;
    };

    class VulkanFrameManager
    {
    public:
        static constexpr uint32_t k_MaxFramesInFlight = 2;

        VulkanFrameManager(const VulkanLogicalDevice &logicalDevice,
                           const VulkanCommandPool &commandPool);
        ~VulkanFrameManager() = default;

        VulkanFrameData &GetCurrentFrame() { return m_Frames[m_CurrentFrameIndex]; }
        const VulkanFrameData &GetCurrentFrame() const { return m_Frames[m_CurrentFrameIndex]; }

        void AdvanceFrame()
        {
            m_CurrentFrameIndex = (m_CurrentFrameIndex + 1) % k_MaxFramesInFlight;
        }

    private:
        VulkanFrameData m_Frames[k_MaxFramesInFlight];
        uint32_t m_CurrentFrameIndex = 0;
    };

} // namespace ve
