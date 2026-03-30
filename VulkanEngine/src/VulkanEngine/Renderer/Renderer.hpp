#pragma once

#include "VulkanEngine/Vulkan/VulkanInstance.hpp"
#include "VulkanEngine/Vulkan/VulkanSurface.hpp"
#include "VulkanEngine/Vulkan/VulkanPhysicalDevice.hpp"
#include "VulkanEngine/Vulkan/VulkanLogicalDevice.hpp"
#include "VulkanEngine/Vulkan/VulkanAllocator.hpp"
#include "VulkanEngine/Vulkan/VulkanSwapchain.hpp"
#include "VulkanEngine/Vulkan/VulkanRenderPass.hpp"
#include "VulkanEngine/Vulkan/VulkanFramebuffers.hpp"

#include <memory>

namespace ve
{
    class Window;

    class Renderer
    {
    public:
        explicit Renderer(const Window &window);
        ~Renderer();

        Renderer(const Renderer &) = delete;
        Renderer &operator=(const Renderer &) = delete;

        bool BeginFrame();
        void EndFrame();

        void HandleResize(uint32_t width, uint32_t height);

        const VulkanLogicalDevice &GetLogicalDeivce() const { return *m_LogicalDevice; }
        const VulkanAllocator &GetAllocator() const { return *m_Allocator; }

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
        std::unique_ptr<VulkanRenderPass> m_RenderPass;
        std::unique_ptr<VulkanFramebuffers> m_Framebuffers;

        // std::unique_ptr<VulkanCommandPool> m_GraphicsCommandPool;
        // std::unique_ptr<VulkanCommandPool> m_TransferCommandPool;

        uint32_t m_CurrentImageIndex = 0;

        bool m_NeedsResize = false;
        uint32_t m_ResizeWidth = 0, m_ResizeHeight = 0;
    };

} // namespace ve
