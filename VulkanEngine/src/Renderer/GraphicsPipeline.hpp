#pragma once

#include <vulkan/vulkan.h>

#include <memory>

namespace VE
{
    class Swapchain;
    class RenderPass;
    class PipelineLayout;

    class GraphicsPipeline final
    {
    public:
        explicit GraphicsPipeline(const Swapchain &swapchain);
        ~GraphicsPipeline();

        VkPipeline Handle() const { return m_Pipeline; }

    private:
        const Swapchain &m_Swapchain;

        VkPipeline m_Pipeline = VK_NULL_HANDLE;

        std::unique_ptr<RenderPass> m_RenderPass;
        std::unique_ptr<PipelineLayout> m_PipelineLayout;
    };

}
