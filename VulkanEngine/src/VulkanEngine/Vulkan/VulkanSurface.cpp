#include "VulkanSurface.hpp"
#include "VulkanInstance.hpp"
#include "Debug/VulkanValidation.hpp"
#include "VulkanEngine/Core/Window.hpp"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace ve
{
    VulkanSurface::VulkanSurface(const VulkanInstance &instance, const ve::Window &window)
        : m_Instance(instance.GetVkHandle())
    {
        auto *glfwWindow = static_cast<GLFWwindow *>(window.GetNativeWindow());

        VkResult result = glfwCreateWindowSurface(m_Instance, glfwWindow, nullptr, &m_Surface);
        CHECK_VK_RESULT(result);
    }

    VulkanSurface::~VulkanSurface()
    {
        if (m_Surface != VK_NULL_HANDLE)
            vkDestroySurfaceKHR(m_Instance, m_Surface, nullptr);
    }

} // namespace ve
