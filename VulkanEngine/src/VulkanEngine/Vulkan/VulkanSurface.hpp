#pragma once

#include <vulkan/vulkan.h>

namespace ve
{
    class Window;

} // namespace ve

namespace ve
{
    class VulkanInstance;

    class VulkanSurface
    {
    public:
        VulkanSurface(const VulkanInstance &instance, const ve::Window &window);
        ~VulkanSurface();

        VulkanSurface(const VulkanSurface &) = delete;
        VulkanSurface &operator=(const VulkanSurface &) = delete;

        VkSurfaceKHR GetVkHandle() const { return m_Surface; }

    private:
        VkSurfaceKHR m_Surface = VK_NULL_HANDLE;
        VkInstance m_Instance = VK_NULL_HANDLE;
    };

} // namespace ve
