#pragma once

#include "VulkanEngine/Vulkan/VulkanInstance.hpp"
#include "VulkanEngine/Vulkan/VulkanSurface.hpp"
#include "VulkanEngine/Vulkan/VulkanPhysicalDevice.hpp"
#include "VulkanEngine/Vulkan/VulkanLogicalDevice.hpp"
#include "VulkanEngine/Vulkan/VulkanAllocator.hpp"
#include "VulkanEngine/Vulkan/VulkanSwapchain.hpp"
#include "VulkanEngine/Vulkan/VulkanRenderPass.hpp"
#include "VulkanEngine/Vulkan/VulkanFramebuffers.hpp"
#include "VulkanEngine/Vulkan/VulkanCommandPool.hpp"
#include "VulkanEngine/Vulkan/VulkanFrameData.hpp"
#include "VulkanEngine/Vulkan/VulkanImmediateSubmit.hpp"
#include "VulkanEngine/Vulkan/VulkanDescriptor.hpp"
#include "VulkanEngine/Vulkan/VulkanPipeline.hpp"

#include "Camera.hpp"

// TODO: remove
#include "Mesh.hpp"
// ---

#include <memory>

namespace ve
{
    class Window;

    struct PushConstants
    {
        glm::mat4 model = glm::mat4(1.0f);
    };

    class Renderer
    {
    public:
        explicit Renderer(const Window &window);
        ~Renderer();

        Renderer(const Renderer &) = delete;
        Renderer &operator=(const Renderer &) = delete;

        void BeginFrame(const Camera &camera);
        void EndFrame();

        void Submit();

        void HandleResize(uint32_t width, uint32_t height);

        const VulkanLogicalDevice &GetLogicalDevice() const { return *m_LogicalDevice; }
        const VulkanAllocator &GetAllocator() const { return *m_Allocator; }

    private:
        void Init(const Window &window);
        void RecreateSwapchain();

    private:
        std::unique_ptr<VulkanInstance> m_Instance;
        std::unique_ptr<VulkanSurface> m_Surface;
        std::unique_ptr<VulkanPhysicalDevice> m_PhysicalDevice;
        std::unique_ptr<VulkanLogicalDevice> m_LogicalDevice;
        std::unique_ptr<VulkanAllocator> m_Allocator;
        std::unique_ptr<VulkanSwapchain> m_Swapchain;
        std::unique_ptr<VulkanRenderPass> m_RenderPass;
        std::unique_ptr<VulkanFramebuffers> m_Framebuffers;
        std::unique_ptr<VulkanCommandPool> m_GraphicsCommandPool;
        std::unique_ptr<VulkanCommandPool> m_TransferCommandPool;
        std::unique_ptr<VulkanFrameManager> m_FrameManager;
        std::unique_ptr<VulkanImmediateSubmit> m_ImmediateSubmit;

        // Pipeline        
        std::unique_ptr<VulkanPipelineLayout> m_PipelineLayout;
        std::unique_ptr<VulkanGraphicsPipeline> m_Pipeline;

        // TODO: remove
        std::unique_ptr<Mesh> m_TriangleMesh;
        // ---

        uint32_t m_CurrentImageIndex = 0;

        bool m_NeedsResize = false;
        uint32_t m_ResizeWidth = 0, m_ResizeHeight = 0;
    };

} // namespace ve
