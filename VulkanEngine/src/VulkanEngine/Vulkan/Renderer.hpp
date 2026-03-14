#pragma once

#include "VulkanEngine/Core/Base.hpp"
#include "VulkanEngine/Vulkan/VulkanContext.hpp"

#include "VulkanSwapchain.hpp"
#include "VulkanRenderPass.hpp"
#include "DescriptorSetManager.hpp"
#include "VulkanPipelineLayout.hpp"
#include "VulkanPipeline.hpp"
#include "VulkanFramebuffer.hpp"
#include "VulkanCommandPool.hpp"
#include "VulkanBuffer.hpp"

#include <vector>
#include <cstdint>

#include <glm/glm.hpp>
#include <stb_image.h>

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
        void CreateDescriptorSetManager();
        void CreateGraphicsPipeline();

        void CreateFramebuffers();

        void CreateCommandPool();
        void CreateCommandBuffers();
        void RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);

        void CreateSyncObjects();

        void CopyBuffer(VkBuffer src, VkBuffer dst, VkDeviceSize size);

        void CreateTextureImage();

        void CreateVertexBuffer();
        void CreateIndexBuffer();
        void CreateUniformBuffers();
        void WriteDescriptorSets();

        void RecreateSwapchain();

    private:
        Ref<VulkanContext> m_Context;
        VkDevice m_Device;

        bool m_FramebufferResized = false;
        VkExtent2D m_FramebufferExtent = {0, 0};
        Scope<VulkanSwapchain> m_Swapchain;

        Scope<VulkanRenderPass> m_RenderPass;
        Scope<DescriptorSetManager> m_DescriptorSetManager;
        Scope<VulkanPipelineLayout> m_PipelineLayout;
        Scope<VulkanPipeline> m_GraphicsPipeline;

        std::vector<Scope<VulkanFramebuffer>> m_Framebuffers;

        Scope<VulkanCommandPool> m_CommandPool;

        std::vector<VkCommandBuffer> m_CommandBuffers;

        VkImage m_TextureImage;
        VkDeviceMemory m_TextureImageMemory;
        Scope<VulkanBuffer> m_VertexBuffer;
        Scope<VulkanDeviceMemory> m_VertexBufferMemory;
        Scope<VulkanBuffer> m_IndexBuffer;
        Scope<VulkanDeviceMemory> m_IndexBufferMemory;

        std::vector<Scope<VulkanBuffer>> m_UniformBuffers;
        std::vector<Scope<VulkanDeviceMemory>> m_UniformBuffersMemory;
        void UpdateUniformBuffers();

        const int MAX_FRAMES_IN_FLIGHT = 3;
        uint32_t m_CurrentFrame = 0;

        std::vector<VkSemaphore> m_ImageAvailableSemaphores;
        std::vector<VkSemaphore> m_RenderFinishedSemaphores;
        std::vector<VkFence> m_InFlightFences;
    };

} // namespace ve
