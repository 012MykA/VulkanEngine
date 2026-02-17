#include "Surface.hpp"
#include "Instance.hpp"
#include "VulkanEngine/Core/Window.hpp"
#include "Validation.hpp"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace VE
{
    Surface::Surface(const Instance &instance, const Window &window) : m_Instance(instance)
    {
        CheckVk(glfwCreateWindowSurface(m_Instance.Handle(), window.Handle(), nullptr, &m_Surface),
                "create window surface!");
    }

    Surface::~Surface()
    {
        vkDestroySurfaceKHR(m_Instance.Handle(), m_Surface, nullptr);
    }

}