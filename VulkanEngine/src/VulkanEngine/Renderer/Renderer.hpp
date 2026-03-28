#pragma once

#include "VulkanEngine/Vulkan/VulkanInstance.hpp"
#include "VulkanEngine/Vulkan/VulkanSurface.hpp"
#include "VulkanEngine/Vulkan/VulkanPhysicalDevice.hpp"
#include "VulkanEngine/Vulkan/VulkanLogicalDevice.hpp"
#include "VulkanEngine/Vulkan/VulkanAllocator.hpp"
#include "VulkanEngine/Vulkan/VulkanSwapchain.hpp"
#include "VulkanEngine/Vulkan/VulkanCommandPool.hpp"
#include "VulkanEngine/Vulkan/VulkanFrameData.hpp"
#include "VulkanEngine/Vulkan/VulkanImmediateSubmit.hpp"
#include "VulkanEngine/Vulkan/VulkanDescriptor.hpp"
#include "VulkanEngine/Vulkan/VulkanPipeline.hpp"

#include <memory>

namespace ve
{
    class Renderer
    {
    public:
        explicit Renderer(const Window &window);
        ~Renderer() = default;

        Renderer(const Renderer &) = delete;
        Renderer &operator=(const Renderer &) = delete;

        bool BeginFrame();
        void EndFrame();

        const VulkanLogicalDevice &GetLogicalDeivce() const { return *m_LogicalDevice; }
        const VulkanAllocator &GetAllocator() const { return *m_Allocator; }
        VulkanImmediateSubmit &GetImmediateSubmit() { return *m_ImmediateSubmit; }

    private:
        void Init(const Window &window);
        void RecreateSwapchain(uint32_t width, uint32_t height);

    private:
        std::unique_ptr<VulkanInstance> m_Instance;
        std::unique_ptr<VulkanSurface> m_Surface;
        std::unique_ptr<VulkanPhysicalDevice> m_PhysicalDevice;
        std::unique_ptr<VulkanLogicalDevice> m_LogicalDevice;
        std::unique_ptr<VulkanAllocator> m_Allocator;

        std::unique_ptr<VulkanSwapchain> m_Swapchain;

        std::unique_ptr<VulkanCommandPool> m_GraphicsCommandPool;
        std::unique_ptr<VulkanCommandPool> m_TransferCommandPool;

        std::unique_ptr<VulkanFrameManager> m_FrameManager;
        std::unique_ptr<VulkanImmediateSubmit> m_ImmediateSubmit;
        std::unique_ptr<VulkanDescriptorPool> m_DescriptorPool;

        uint32_t m_CurrentImageIndex = 0;
    };

} // namespace ve
