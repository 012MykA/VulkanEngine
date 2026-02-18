#pragma once

#include <vulkan/vulkan.h>

namespace VE
{
    class Device;
    class RenderPass;

    class Framebuffer final
    {
    public:
        Framebuffer(const Framebuffer &) = delete;
        Framebuffer &operator=(const Framebuffer &) = delete;
        Framebuffer &operator=(Framebuffer &&) = delete;

        Framebuffer(const Device &device,
                    const RenderPass &renderPass,
                    VkImageView imageView,
                    VkExtent2D extent);
        Framebuffer(Framebuffer &&other) noexcept;
        ~Framebuffer();

        VkFramebuffer Handle() const { return m_Framebuffer; }

    private:
        const Device &m_Device;

        VkFramebuffer m_Framebuffer = VK_NULL_HANDLE;
    };

}
