#pragma once

#include <vulkan/vulkan.h>

namespace ve
{
    class VulkanInstance;
    class Window;

    class VulkanSurface
    {
    public:
        VulkanSurface(const VulkanInstance &instance, const ve::Window &window);
        ~VulkanSurface();

        VulkanSurface(const VulkanSurface &) = delete;
        VulkanSurface &operator=(const VulkanSurface &) = delete;

        VkSurfaceKHR GetVkHandle() const { return m_Surface; }

    private:
        VkInstance m_Instance = VK_NULL_HANDLE;
        VkSurfaceKHR m_Surface = VK_NULL_HANDLE;
    };

} // namespace ve
