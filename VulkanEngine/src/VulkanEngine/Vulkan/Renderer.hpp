#pragma once

#include "VulkanEngine/Core/Base.hpp"
#include "VulkanEngine/Vulkan/VulkanContext.hpp"

#include "VulkanRenderPass.hpp"
#include "VulkanPipelineLayout.hpp"
#include "VulkanPipeline.hpp"
#include "VulkanFramebuffer.hpp"
#include "VulkanCommandPool.hpp"
#include "VulkanBuffer.hpp"

#include <vector>
#include <cstdint>

namespace ve
{
    class Renderer
    {
    public:
        Renderer(VulkanContext *context);
        ~Renderer();

        void DrawFrame();

    private:
        void CreateGraphicsPipeline();
        void CreateFramebuffers();

        void CreateCommandPool();
        void CreateCommandBuffers();
        void RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);

        void CreateSyncObjects();

        void CreateVertexBuffer();
        void CreateIndexBuffer();
        void CopyBuffer(VkBuffer src, VkBuffer dst, VkDeviceSize size);

        void RecreateSwapchain();

    private:
        VulkanContext *m_Context;
        VkDevice m_Device;

        Scope<VulkanRenderPass> m_RenderPass;
        Scope<VulkanPipelineLayout> m_PipelineLayout;
        Scope<VulkanPipeline> m_GraphicsPipeline;

        std::vector<Scope<VulkanFramebuffer>> m_Framebuffers;

        Scope<VulkanCommandPool> m_CommandPool;

        std::vector<VkCommandBuffer> m_CommandBuffers;

        Scope<VulkanBuffer> m_VertexBuffer;
        Scope<VulkanBuffer> m_IndexBuffer;

        const int MAX_FRAMES_IN_FLIGHT = 3;
        uint32_t m_CurrentFrame = 0;

        std::vector<VkSemaphore> m_ImageAvailableSemaphores;
        std::vector<VkSemaphore> m_RenderFinishedSemaphores;
        std::vector<VkFence> m_InFlightFences;
    };

} // namespace ve
