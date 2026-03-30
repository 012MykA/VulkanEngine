#include "Renderer.hpp"
#include "VulkanEngine/Core/Window.hpp"

namespace ve
{
    Renderer::Renderer(const Window &window)
    {
        Init(window);
    }

    bool Renderer::BeginFrame()
    {
        return false;
    }

    void Renderer::EndFrame()
    {
    }

    void Renderer::HandleResize(uint32_t width, uint32_t height)
    {
    }

    void Renderer::Init(const Window &window)
    {
        // Instance
        m_Instance = std::make_unique<VulkanInstance>(InstanceDesc{
            .requiredExtensions = window.GetRequiredVulkanExtensions(),
#ifdef VE_DEBUG
            .enableValidation = true,
            .debugMessenger{
                .enableDebugMessenger = true,
                .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                   VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
            },
#endif
        });

        m_Surface = std::make_unique<VulkanSurface>(*m_Instance, window);
    }

    void Renderer::RecreateSwapchain(uint32_t width, uint32_t height)
    {
    }

} // namespace ve
