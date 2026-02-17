#pragma once

#include <vulkan/vulkan.h>

#include <memory>
#include <vector>

namespace VE
{
    class Window;
    class Instance;
    class Surface;
    class Device;
    class Swapchain;
    class RenderPass;
    class GraphicsPipeline;
    class Framebuffer;
    class CommandPool;
    class CommandBuffers;

    class Renderer final
    {
    public:
        Renderer(const Instance &instance, const Surface &surface, const Window &window);
        ~Renderer();

        void DrawFrame();

    private:
        void CreateFramebuffers();
        void CreateCommandBuffers();

    private:
        std::unique_ptr<Device> m_Device;
        std::unique_ptr<Swapchain> m_Swapchain;

        std::unique_ptr<RenderPass> m_RenderPass;
        std::unique_ptr<GraphicsPipeline> m_Pipeline;

        std::unique_ptr<CommandPool> m_CommandPool;
        std::unique_ptr<CommandBuffers> m_CommandBuffers;

        std::vector<std::unique_ptr<Framebuffer>> m_Framebuffers;
    };

}
