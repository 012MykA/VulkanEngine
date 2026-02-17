#include "Renderer.hpp"
#include "Window.hpp"
#include "Instance.hpp"
#include "Surface.hpp"
#include "Device.hpp"
#include "Swapchain.hpp"
#include "RenderPass.hpp"
#include "GraphicsPipeline.hpp"
#include "Framebuffer.hpp"
#include "CommandPool.hpp"
#include "CommandBuffers.hpp"

namespace VE
{
    Renderer::Renderer(const Instance &instance, const Surface &surface, const Window &window)
    {
        m_Device = std::make_unique<Device>(instance, surface);
        m_Swapchain = std::make_unique<Swapchain>(*m_Device, surface, window);

        m_RenderPass = std::make_unique<RenderPass>(*m_Swapchain);
        m_Pipeline = std::make_unique<GraphicsPipeline>(*m_Swapchain);

        CreateFramebuffers();

        m_CommandPool = std::make_unique<CommandPool>(*m_Device);
        CreateCommandBuffers();
    }

    Renderer::~Renderer() = default;

    void Renderer::DrawFrame()
    {
    }

    void Renderer::CreateFramebuffers()
    {
        for (auto imageView : m_Swapchain->GetImageViews())
        {
            m_Framebuffers.push_back(
                std::make_unique<Framebuffer>(
                    *m_Device,
                    *m_RenderPass,
                    imageView,
                    m_Swapchain->GetExtent()));
        }
    }

    void Renderer::CreateCommandBuffers()
    {
        m_CommandBuffers = std::make_unique<CommandBuffers>(*m_Device, *m_CommandPool, static_cast<uint32_t>(m_Framebuffers.size()));
    }

}
