#pragma once

#include "VulkanEngine/Vulkan/VulkanInstance.hpp"
#include "VulkanEngine/Vulkan/VulkanSurface.hpp"
#include "VulkanEngine/Vulkan/VulkanPhysicalDevice.hpp"
#include "VulkanEngine/Vulkan/VulkanLogicalDevice.hpp"
#include "VulkanEngine/Vulkan/VulkanSwapchain.hpp"
#include "VulkanEngine/Vulkan/VulkanCommandPool.hpp"
#include "VulkanEngine/Vulkan/VulkanFrameData.hpp"

#include <memory>

namespace ve
{
    class Renderer
    {
    public:
        Renderer();
        ~Renderer();

        
    
    private:
        std::unique_ptr<VulkanInstance> m_Instance;
        std::unique_ptr<VulkanSurface> m_Surface;
        std::unique_ptr<VulkanPhysicalDevice> m_PhysicalDevice;
        std::unique_ptr<VulkanLogicalDevice> m_LogicalDevice;
        std::unique_ptr<VulkanSwapchain> m_Swapchain;
        std::unique_ptr<VulkanCommandPool> m_GraphicsCommandPool;
        std::unique_ptr<VulkanCommandPool> m_TransferCommandPool;
        std::unique_ptr<VulkanFrameManager> m_FrameManager;
        uint32_t m_CurrentImageIndex = 0;
    };

} // namespace ve
