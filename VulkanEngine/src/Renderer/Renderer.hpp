#pragma once
#include "Framebuffer.hpp"
#include "Semaphore.hpp"
#include "Fence.hpp"
#include "UniformBuffer.hpp"

// TODO: remove
#include "Vertex.hpp"
#include "DescriptorBinding.hpp"
// ---

#include <vulkan/vulkan.h>

#include <memory>
#include <vector>
#include <cstdint>

namespace VE
{
    class Window;
    class Instance;
    class Surface;
    class Device;
    class Swapchain;
    class RenderPass;
    class DescriptorSetLayout;
    class GraphicsPipeline;
    class Framebuffer;
    class CommandPool;
    class CommandBuffers;
    class Semaphore;
    class Fence;
    class DepthBuffer;
    // TODO: remove
    class Buffer;
    class DeviceMemory;
    class DescriptorPool;
    class DescriptorSets;
    class TextureImage;
    class Model;
    // ---

    class Renderer final
    {
    public:
        Renderer(const Instance &instance, const Surface &surface, const Window &window);
        ~Renderer();

        void DrawFrame();

        void OnResize();

        const Device &GetDevice() const { return *m_Device; }

    private:
        void CreateCommandPool();
        void CreateSwapchain();
        void CreateDepthBuffer();
        void CreateRenderPass();
        void CreateDescriptorSetLayout();
        void CreateGraphicsPipeline();
        void CreateFramebuffers();

        // TODO: remove
        void CreateTextureImage();
        void LoadModel();
        void CreateVertexBuffer();
        void CreateIndexBuffer();
        void CreateUniformBuffers();
        // ---

        void CreateDescriptorPool();
        void CreateDescriptorSets();
        
        void CreateCommandBuffers();
        void CreateSyncObjects();

        void RecreateSwapchain();
        void UpdateUniformBuffer(uint32_t currentFrame);

    private:
        const Window &m_Window;
        const Surface &m_Surface;

        std::unique_ptr<Device> m_Device;
        std::unique_ptr<CommandPool> m_CommandPool;
        std::unique_ptr<Swapchain> m_Swapchain;
        std::unique_ptr<DepthBuffer> m_DepthBuffer;
        std::unique_ptr<RenderPass> m_RenderPass;

        // TODO: remove
        std::vector<DescriptorBinding> m_DescriptorBindings;
        std::unique_ptr<DescriptorSetLayout> m_DescriptorSetLayout;
        // ---

        std::unique_ptr<GraphicsPipeline> m_Pipeline;
        std::vector<Framebuffer> m_Framebuffers;

    private:
        // TODO: remove
        std::unique_ptr<TextureImage> m_TextureImage;
        std::unique_ptr<Model> m_Model;

        std::vector<Vertex> m_Vertices;
        std::unique_ptr<Buffer> m_VertexBuffer;
        std::unique_ptr<DeviceMemory> m_VertexBufferMemory;

        std::vector<uint32_t> m_Indices;
        std::unique_ptr<Buffer> m_IndexBuffer;
        std::unique_ptr<DeviceMemory> m_IndexBufferMemory;

        std::vector<UniformBuffer> m_UniformBuffers;
        // ---

    private:
        std::unique_ptr<DescriptorPool> m_DescriptorPool;
        std::unique_ptr<DescriptorSets> m_DescriptorSets;

        std::unique_ptr<CommandBuffers> m_CommandBuffers;

        // Sync objects
        static constexpr int MAX_FRAMES_IN_FLIGHT = 3;
        std::vector<Semaphore> m_ImageAvailableSemaphores;
        std::vector<Semaphore> m_RenderFinishedSemaphores;
        std::vector<Fence> m_InFlightFences;
        uint32_t currentFrame = 0;

        bool m_FramebufferResized = false;
    };

}
