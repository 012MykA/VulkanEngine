#pragma once

#include "VulkanEngine/Core/Base.hpp"
#include "VulkanEngine/Vulkan/VulkanContext.hpp"

#include "VulkanSwapchain.hpp"
#include "VulkanRenderPass.hpp"
#include "VulkanPipelineLayout.hpp"
#include "VulkanPipeline.hpp"
#include "VulkanFramebuffer.hpp"
#include "VulkanCommandPool.hpp"
#include "VulkanBuffer.hpp"

#include <vector>
#include <cstdint>

#include <glm/glm.hpp>

namespace ve
{
    struct UniformBufferObject
    {
        glm::mat4 model;
        glm::mat4 view;
        glm::mat4 proj;
    };

    class Renderer
    {
    public:
        Renderer(Ref<VulkanContext> context, VkExtent2D windowExtent);
        ~Renderer();

        void DrawFrame();

        void OnWindowResize(uint32_t width, uint32_t height);

    private:
        void CreateDescriptorSetLayout();

        void CreateGraphicsPipeline();
        void CreateFramebuffers();

        void CreateCommandPool();
        void CreateCommandBuffers();
        void RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);

        void CreateSyncObjects();

        void CreateVertexBuffer();
        void CreateIndexBuffer();
        void CreateUniformBuffers();
        void CopyBuffer(VkBuffer src, VkBuffer dst, VkDeviceSize size);

        void RecreateSwapchain();

    private:
        Ref<VulkanContext> m_Context;
        VkDevice m_Device;

        bool m_FramebufferResized = false;
        VkExtent2D m_FramebufferExtent = {0, 0};
        Scope<VulkanSwapchain> m_Swapchain;

        Scope<VulkanRenderPass> m_RenderPass;
        Scope<VulkanPipelineLayout> m_PipelineLayout;
        Scope<VulkanPipeline> m_GraphicsPipeline;

        std::vector<Scope<VulkanFramebuffer>> m_Framebuffers;

        Scope<VulkanCommandPool> m_CommandPool;

        std::vector<VkCommandBuffer> m_CommandBuffers;

        Scope<VulkanBuffer> m_VertexBuffer;
        Scope<VulkanBuffer> m_IndexBuffer;
        VkDescriptorSetLayout m_DescriptorSetLayout = VK_NULL_HANDLE;
        std::vector<Scope<VulkanBuffer>> m_UniformBuffers;

        const int MAX_FRAMES_IN_FLIGHT = 3;
        uint32_t m_CurrentFrame = 0;

        std::vector<VkSemaphore> m_ImageAvailableSemaphores;
        std::vector<VkSemaphore> m_RenderFinishedSemaphores;
        std::vector<VkFence> m_InFlightFences;
    };

} // namespace ve
