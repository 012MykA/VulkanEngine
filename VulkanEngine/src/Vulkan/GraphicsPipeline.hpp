#pragma once

#include <vulkan/vulkan.h>

#include <memory>

namespace VE
{
    class Swapchain;
    class RenderPass;
    class DescriptorSetLayout;
    class PipelineLayout;

    class GraphicsPipeline final
    {
    public:
        GraphicsPipeline(const Swapchain &swapchain, const RenderPass &renderPass, const DescriptorSetLayout &descriptorSetLayout);
        ~GraphicsPipeline();

        VkPipeline Handle() const { return m_Pipeline; }
        const PipelineLayout& Layout() const { return *m_PipelineLayout; } 

    private:
        const Swapchain &m_Swapchain;
        const RenderPass &m_RenderPass;

        VkPipeline m_Pipeline = VK_NULL_HANDLE;

        std::unique_ptr<PipelineLayout> m_PipelineLayout;
    };

}
