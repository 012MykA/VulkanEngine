#include "Window.hpp"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <stb_image.h>

#include <stdexcept>
#include <iostream>

namespace VE
{
    namespace
    {
        void GlfwErrorCallback(const int error, const char *const description)
        {
            // TODO: replace with logging system
            std::cerr << "ERROR: GLFW: " << description << " (code: " << error << ")" << std::endl;
        }

    }

    Window::Window(const WindowConfig &config) : m_Config(config)
    {
        glfwSetErrorCallback(GlfwErrorCallback);

        if (!glfwInit())
            throw std::runtime_error("failed to initialize GLFW!");

        if (!glfwVulkanSupported())
            throw std::runtime_error("Vulkan is not supported by GLFW!");

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, m_Config.Resizable ? GLFW_TRUE : GLFW_FALSE);

        GLFWmonitor *const monitor = config.Fullscreen ? glfwGetPrimaryMonitor() : nullptr;

        m_Window = glfwCreateWindow(m_Config.Width, m_Config.Height, m_Config.Title.c_str(), monitor, nullptr);
        if (!m_Window)
            throw std::runtime_error("failed to create GLFW window!");

        GLFWimage icon;
        icon.pixels = stbi_load("VulkanEngine/Assets/Textures/Vulkan.png", &icon.width, &icon.height, nullptr, 4);
        if (icon.pixels == nullptr)
            throw std::runtime_error("failed to window icon");

        glfwSetWindowIcon(m_Window, 1, &icon);
        stbi_image_free(icon.pixels);

        glfwSetWindowUserPointer(m_Window, this);
    }

    Window::~Window()
    {
        glfwDestroyWindow(m_Window);

        glfwTerminate();
    }

    std::vector<const char *> Window::GetRequiredInstanceExtensions() const
    {
        uint32_t glfwExtensionCount = 0;
        const char **glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
        return std::vector<const char *>(glfwExtensions, glfwExtensions + glfwExtensionCount);
    }

    void Window::OnUpdate()
    {
        glfwPollEvents();
    }

    bool Window::ShouldClose() const
    {
        return glfwWindowShouldClose(m_Window);
    }

}
