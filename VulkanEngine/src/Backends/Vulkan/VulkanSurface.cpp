#include "VulkanSurface.hpp"
#include "VulkanInstance.hpp"
#include "Debug/VulkanValidation.hpp"
#include "VulkanEngine/Core/Window.hpp"

namespace ve
{
    VulkanSurface::VulkanSurface(const VulkanInstance &instance, const ve::Window &window)
        : m_Instance(instance.GetVkHandle()), m_Surface(window.GetVulkanSurface(m_Instance))
    {
    }

    VulkanSurface::~VulkanSurface()
    {
        if (m_Surface != VK_NULL_HANDLE)
            vkDestroySurfaceKHR(m_Instance, m_Surface, nullptr);
    }

} // namespace ve
