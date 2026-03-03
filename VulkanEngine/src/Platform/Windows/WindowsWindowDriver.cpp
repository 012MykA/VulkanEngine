#include "VulkanEngine/Core/Base.hpp"
#ifdef VE_PLATFORM_WINDOWS

#include "WindowsWindowDriver.hpp"
#include "VulkanEngine/Core/Log.hpp"
#include "VulkanEngine/Events/ApplicationEvent.hpp"
#include "VulkanEngine/Events/KeyEvent.hpp"
#include "VulkanEngine/Events/MouseEvent.hpp"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <stb_image.h>

#include <stdexcept>

namespace ve
{
    namespace
    {
        void GlfwErrorCallback(const int error, const char *const description)
        {
            VE_CORE_ERROR("GLFW Error ({0}): {1}", error, description);
        }
    }

    WindowsWindowDriver::WindowsWindowDriver(const WindowCreateInfo &createInfo)
    {
        m_Data.Title = createInfo.Title;
        m_Data.Width = createInfo.Width;
        m_Data.Height = createInfo.Height;

        glfwSetErrorCallback(GlfwErrorCallback);

        if (!glfwInit())
            throw std::runtime_error("failed to initialize GLFW!");

        if (!glfwVulkanSupported())
            throw std::runtime_error("Vulkan is not supported by GLFW!");

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, createInfo.Resizable ? GLFW_TRUE : GLFW_FALSE);

        GLFWmonitor *const monitor = createInfo.Fullscreen ? glfwGetPrimaryMonitor() : nullptr;

        m_Window = glfwCreateWindow(createInfo.Width, createInfo.Height, createInfo.Title.c_str(), monitor, nullptr);
        if (!m_Window)
            throw std::runtime_error("failed to create GLFW window!");

        if (!createInfo.IconPath.empty())
        {
            GLFWimage icon;
            icon.pixels = stbi_load(createInfo.IconPath.string().c_str(), &icon.width, &icon.height, nullptr, STBI_rgb_alpha);
            if (icon.pixels == nullptr)
                throw std::runtime_error("failed to load window icon");

            glfwSetWindowIcon(m_Window, 1, &icon);
            stbi_image_free(icon.pixels);
        }

        glfwSetWindowUserPointer(m_Window, &m_Data);

        // Set GLFW callbacks
        glfwSetWindowSizeCallback(m_Window, [](GLFWwindow *window, int width, int height)
                                  {
            WindowData& data = *reinterpret_cast<WindowData*>(glfwGetWindowUserPointer(window));
            data.Width = static_cast<uint32_t>(width);
            data.Height = static_cast<uint32_t>(height);

            WindowResizeEvent event(width, height);
            data.EventCallback(event); });

        glfwSetWindowCloseCallback(m_Window, [](GLFWwindow *window)
                                   {
            WindowData& data =*reinterpret_cast<WindowData*>(glfwGetWindowUserPointer(window));

            WindowCloseEvent event;
            data.EventCallback(event); });

        glfwSetKeyCallback(m_Window, [](GLFWwindow *window, int key, int scancode, int action, int mods)
                           {
            WindowData& data =*reinterpret_cast<WindowData*>(glfwGetWindowUserPointer(window));

            switch (action)
            {
                case GLFW_PRESS:
                {
                    KeyPressedEvent event(key, 0);
                    data.EventCallback(event);
                    break;
                }
                case GLFW_RELEASE:
                {
                    KeyReleasedEvent event(key);
                    data.EventCallback(event);
                    break;
                }
                case GLFW_REPEAT:
                {
                    KeyPressedEvent event(key, 1);
                    data.EventCallback(event);
                    break;
                }
            } });

        glfwSetCharCallback(m_Window, [](GLFWwindow *window, unsigned int keycode)
                            {
            WindowData& data =*reinterpret_cast<WindowData*>(glfwGetWindowUserPointer(window));
            KeyTypedEvent event(keycode);
            data.EventCallback(event); });

        glfwSetMouseButtonCallback(m_Window, [](GLFWwindow *window, int button, int action, int mods)
                                   {
            WindowData& data =*reinterpret_cast<WindowData*>(glfwGetWindowUserPointer(window));

            switch (action)
            {
                case GLFW_PRESS:
                {
                    MouseButtonPressedEvent event(button);
                    data.EventCallback(event);
                    break;
                }
                case GLFW_RELEASE:
                {
                    MouseButtonReleasedEvent event(button);
                    data.EventCallback(event);
                    break;
                }
            } });

        glfwSetScrollCallback(m_Window, [](GLFWwindow *window, double xOffset, double yOffset)
                              {
            WindowData& data = *reinterpret_cast<WindowData*>(glfwGetWindowUserPointer(window));
            
            MouseScrolledEvent event(static_cast<float>(xOffset), static_cast<float>(yOffset));
            data.EventCallback(event); });

        glfwSetCursorPosCallback(m_Window, [](GLFWwindow *window, double xPos, double yPos)
                                 {
            WindowData& data = *reinterpret_cast<WindowData*>(glfwGetWindowUserPointer(window));

            MouseMovedEvent event(static_cast<float>(xPos), static_cast<float>(yPos));
            data.EventCallback(event); });
    }

    WindowsWindowDriver::~WindowsWindowDriver()
    {
        glfwDestroyWindow(m_Window);
        glfwTerminate();
    }

    void WindowsWindowDriver::OnUpdate()
    {
        glfwPollEvents();
    }

    uint32_t WindowsWindowDriver::GetWidth() const
    {
        return m_Data.Width;
    }

    uint32_t WindowsWindowDriver::GetHeight() const
    {
        return m_Data.Height;
    }

    void WindowsWindowDriver::SetEventCallback(const EventCallbackFn &callback)
    {
        m_Data.EventCallback = callback;
    }

} // namespace ve

#endif // VE_PLATFORM_WINDOWS
