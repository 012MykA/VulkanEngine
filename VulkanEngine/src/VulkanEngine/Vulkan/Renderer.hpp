#pragma once

#include "VulkanContext.hpp"
#include "VulkanRenderPass.hpp"
#include "VulkanPipelineLayout.hpp"
#include "VulkanPipeline.hpp"

// TODO: remove
#include "Vertex.hpp"
#include "VulkanBuffer.hpp"
// ---

namespace ve
{
    class Renderer
    {
    public:
        Renderer(VulkanContext* context);
        ~Renderer();

        void DrawFrame();

    private:
        VulkanContext *m_Context;
    };

} // namespace ve
