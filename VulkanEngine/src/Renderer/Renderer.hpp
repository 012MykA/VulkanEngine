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
    // TODO: remove
    class Buffer;
    class DeviceMemory;
    class DescriptorPool;
    class DescriptorSets;
    class Image;
    class ImageView;
    class Sampler;
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
        void CreateSwapchain();
        void CreateFramebuffers();
        void CreateCommandBuffers();
        void CreateSyncObjects();

        void RecreateSwapchain();
        void UpdateUniformBuffer(uint32_t currentFrame);

    private:
        const Surface &m_Surface;
        const Window &m_Window;

        std::unique_ptr<Device> m_Device;
        std::unique_ptr<Swapchain> m_Swapchain;

        std::unique_ptr<RenderPass> m_RenderPass;

        // TODO: remove
        std::unique_ptr<DescriptorSetLayout> m_DescriptorSetLayout;
        std::vector<DescriptorBinding> m_DescriptorBindings;
        void CreateDescriptorSetLayout();
        // ---

        std::unique_ptr<GraphicsPipeline> m_Pipeline;

        std::unique_ptr<CommandPool> m_CommandPool;

        // TODO: remove
        std::unique_ptr<Image> m_TextureImage;
        std::unique_ptr<DeviceMemory> m_TextureImageMemory;
        void CreateTextureImage();
        std::unique_ptr<ImageView> m_TextureImageView;
        std::unique_ptr<Sampler> m_TextureSampler;
        // ---

        std::unique_ptr<CommandBuffers> m_CommandBuffers;
        std::vector<Framebuffer> m_Framebuffers;

        // Sync objects
        static constexpr int MAX_FRAMES_IN_FLIGHT = 3;
        std::vector<Semaphore> m_ImageAvailableSemaphores;
        std::vector<Semaphore> m_RenderFinishedSemaphores;
        std::vector<Fence> m_InFlightFences;
        uint32_t currentFrame = 0;

        bool m_FramebufferResized = false;

    private:
        // TODO: remove
        std::vector<Vertex> m_Vertices;
        std::unique_ptr<Buffer> m_VertexBuffer;
        std::unique_ptr<DeviceMemory> m_VertexBufferMemory;
        void CreateVertexBuffer();

        std::vector<uint16_t> m_Indices;
        std::unique_ptr<Buffer> m_IndexBuffer;
        std::unique_ptr<DeviceMemory> m_IndexBufferMemory;
        void CreateIndexBuffer();

        std::vector<UniformBuffer> m_UniformBuffers;
        void CreateUniformBuffers();

        std::unique_ptr<DescriptorPool> m_DescriptorPool;

        std::unique_ptr<DescriptorSets> m_DescriptorSets;
        void CreateDescriptorSets();
        // ---
    };

}
