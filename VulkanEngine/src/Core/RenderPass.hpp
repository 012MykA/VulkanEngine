#pragma once

#include <vulkan/vulkan.h>

namespace VE
{
    class Swapchain;

    class RenderPass final
    {
    public:
        RenderPass(const Swapchain &swapchain);
        ~RenderPass();

        VkRenderPass Handle() const { return m_RenderPass; }

    private:
        const Swapchain &m_Swapchain;
        VkRenderPass m_RenderPass = VK_NULL_HANDLE;
    };

}
