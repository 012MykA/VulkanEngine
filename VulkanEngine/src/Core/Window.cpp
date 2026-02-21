#include "Window.hpp"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <stb_image.h>

#include <stdexcept>
#include <iostream>
#include <string>

namespace VE
{
    static const std::string ICON_PATH = "assets/textures/vulkan.png";
    
    namespace
    {
        void GlfwErrorCallback(const int error, const char *const description)
        {
            // TODO: replace with logging system
            std::cerr << "ERROR: GLFW: " << description << " (code: " << error << ")" << std::endl;
        }

        void GlfwFramebufferResizeCallback(GLFWwindow *window, int width, int height)
        {
            auto *const this_ = static_cast<Window *>(glfwGetWindowUserPointer(window));
            if (this_->OnResize)
            {
                this_->OnResize(width, height);
            }
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
        icon.pixels = stbi_load(ICON_PATH.c_str(), &icon.width, &icon.height, nullptr, STBI_rgb_alpha);
        if (icon.pixels == nullptr)
            throw std::runtime_error("failed to window icon");

        glfwSetWindowIcon(m_Window, 1, &icon);
        stbi_image_free(icon.pixels);

        glfwSetWindowUserPointer(m_Window, this);
        glfwSetFramebufferSizeCallback(m_Window, GlfwFramebufferResizeCallback);
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

    bool Window::IsMinimized() const
    {
        int width, height;
        glfwGetFramebufferSize(m_Window, &width, &height);
        return width == 0 || height == 0;
    }

    void Window::WaitForEvents() const
    {
        glfwWaitEvents();
    }

    bool Window::ShouldClose() const
    {
        return glfwWindowShouldClose(m_Window);
    }

}
