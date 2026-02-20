#pragma once

#include <vulkan/vulkan.h>

namespace VE
{
    class Swapchain;
    class DepthBuffer;

    class RenderPass final
    {
    public:
        explicit RenderPass(const Swapchain &swapchain, const DepthBuffer &depthBuffer);
        ~RenderPass();

        VkRenderPass Handle() const { return m_RenderPass; }
        const DepthBuffer &GetDepthBuffer() const { return m_DepthBuffer; };

    private:
        const Swapchain &m_Swapchain;
        const DepthBuffer &m_DepthBuffer;
        VkRenderPass m_RenderPass = VK_NULL_HANDLE;
    };

}
