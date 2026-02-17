#pragma once

#include <vulkan/vulkan.h>

namespace VE
{
    class Device;
    class RenderPass;

    class Framebuffer
    {
    public:
        Framebuffer(const Device &device,
                    const RenderPass &renderPass,
                    VkImageView imageView,
                    VkExtent2D extent);
        ~Framebuffer();

        VkFramebuffer Handle() const { return m_Framebuffer; }

    private:
        const Device &m_Device;

        VkFramebuffer m_Framebuffer;
    };

}
