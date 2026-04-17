#include "VulkanFrameManager.hpp"
#include "VulkanLogicalDevice.hpp"
#include "VulkanCommandPool.hpp"
#include "VulkanEngine/Core/Log.hpp"

namespace ve
{
    VulkanFrameManager::VulkanFrameManager(const VulkanLogicalDevice &logicalDevice, const VulkanCommandPool &commandPool)
    {
        auto buffers = commandPool.AllocateBuffers(k_MaxFramesInFlight);

        for (uint32_t i = 0; i < k_MaxFramesInFlight; i++)
        {
            m_Frames[i].commandBuffer = buffers[i];
            m_Frames[i].syncObjects = std::make_unique<VulkanSyncObjects>(logicalDevice);
        }

        VE_CORE_TRACE("FrameManager created:");
        VE_CORE_TRACE("  frames-in-flight: {}", k_MaxFramesInFlight);
    }

} // namespace ve
