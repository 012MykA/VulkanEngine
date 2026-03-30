#include "Renderer.hpp"
#include "VulkanEngine/Core/Window.hpp"
#include "VulkanEngine/Core/Log.hpp"
#include "VulkanEngine/Core/Timer.hpp"

namespace ve
{
    Renderer::Renderer(const Window &window)
    {
        Init(window);
    }

    Renderer::~Renderer()
    {
        m_LogicalDevice->WaitIdle();
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
        if (width == 0 || height == 0)
            return;

        m_LogicalDevice->WaitIdle();

        m_NeedsResize = true;
        m_ResizeWidth = width;
        m_ResizeHeight = height;

        RecreateSwapchain(width, height);
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
        m_PhysicalDevice = std::make_unique<VulkanPhysicalDevice>(*m_Instance, *m_Surface);
        m_LogicalDevice = std::make_unique<VulkanLogicalDevice>(*m_PhysicalDevice, LogicalDeviceDesc{});
        m_Allocator = std::make_unique<VulkanAllocator>(*m_Instance, *m_PhysicalDevice, *m_LogicalDevice);

        m_Swapchain = std::make_unique<VulkanSwapchain>(
            *m_PhysicalDevice,
            *m_LogicalDevice,
            *m_Surface,
            SwapchainDesc{
                .width = window.GetWidth(),
                .height = window.GetHeight(),
            });
    }

    void Renderer::RecreateSwapchain(uint32_t width, uint32_t height)
    {
        Timer timer;
        
        m_Swapchain->Recreate(width, height);

        VE_CORE_TRACE("Swapchain recreated: {}x{} ({} ms)", width, height, timer.ElapsedMilliseconds());
    }

} // namespace ve
