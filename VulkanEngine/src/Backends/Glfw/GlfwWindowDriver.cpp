#include "GlfwWindowDriver.hpp"
#include "VulkanEngine/Core/Log.hpp"
#include "VulkanEngine/Events/ApplicationEvent.hpp"
#include "VulkanEngine/Events/KeyEvent.hpp"
#include "VulkanEngine/Events/MouseEvent.hpp"
#include "Backends/Vulkan/Debug/VulkanValidation.hpp"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <stb_image.h>

#include <stdexcept>

namespace ve
{
    static bool s_GlfwInitialized = false; // TODO: Add glfw context in future

    namespace
    {
        void GlfwErrorCallback(const int error, const char *const description)
        {
            VE_CORE_ERROR("GLFW Error ({0}): {1}", error, description);
        }
    }

    GlfwWindowDriver::GlfwWindowDriver(const WindowDesc &desc)
        : m_IsFullscreen(desc.fullscreen), m_LastPosX(desc.posX), m_LastPosY(desc.posY),
          m_LastWidth(desc.width), m_LastHeight(desc.height)
    {
        m_Data.title = desc.title;
        m_Data.width = desc.width;
        m_Data.height = desc.height;

        // Glfw error callback
        glfwSetErrorCallback(GlfwErrorCallback);

        // Initializing glfw
        if (!s_GlfwInitialized)
        {
            int success = glfwInit();
            if (!success)
                throw std::runtime_error("failed to initialize GLFW!");
            s_GlfwInitialized = true;
        }

        // Check Vulkan support
        if (!glfwVulkanSupported())
            throw std::runtime_error("Vulkan is not supported by GLFW!");

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, desc.resizable ? GLFW_TRUE : GLFW_FALSE);

        // Handle Fullscreen
        GLFWmonitor *monitor = desc.fullscreen ? glfwGetPrimaryMonitor() : nullptr;

        m_WindowHandle = glfwCreateWindow(desc.width, desc.height, desc.title.c_str(), monitor, nullptr);
        if (!m_WindowHandle)
            throw std::runtime_error("failed to create GLFW window!");

        // Set Window Pos
        if (!desc.fullscreen)
        {
            if (desc.centered)
            {
                monitor = glfwGetPrimaryMonitor();
                const GLFWvidmode *mode = glfwGetVideoMode(monitor);

                if (mode)
                {
                    int xpos = (mode->width - static_cast<int>(desc.width)) / 2;
                    int ypos = (mode->height - static_cast<int>(desc.height)) / 2;

                    glfwSetWindowPos(m_WindowHandle, xpos, ypos);
                }
            }
            else
            {
                glfwSetWindowPos(m_WindowHandle, static_cast<int>(desc.posX), static_cast<int>(desc.posY));
            }
        }

        // Set Window Icon
        if (!desc.iconPath.empty())
        {
            GLFWimage icon;
            icon.pixels = stbi_load(desc.iconPath.string().c_str(), &icon.width, &icon.height, nullptr, STBI_rgb_alpha);

            if (icon.pixels == nullptr)
            {
                VE_CORE_ERROR("failed to load window icon: {0}", desc.iconPath);
            }
            else
            {
                glfwSetWindowIcon(m_WindowHandle, 1, &icon);
                stbi_image_free(icon.pixels);
            }
        }

        // User pointer
        glfwSetWindowUserPointer(m_WindowHandle, &m_Data);

        // clang-format off

        // Set GLFW callbacks
        glfwSetWindowSizeCallback(m_WindowHandle, [](GLFWwindow *window, int width, int height) {
            WindowData& data = *reinterpret_cast<WindowData*>(glfwGetWindowUserPointer(window));
            data.width = static_cast<uint32_t>(width);
            data.height = static_cast<uint32_t>(height);

            WindowResizeEvent event(width, height);
            data.eventCallback(event);
        });

        glfwSetWindowCloseCallback(m_WindowHandle, [](GLFWwindow *window) {
            WindowData& data =*reinterpret_cast<WindowData*>(glfwGetWindowUserPointer(window));

            WindowCloseEvent event;
            data.eventCallback(event);
        });

        glfwSetKeyCallback(m_WindowHandle, [](GLFWwindow *window, int key, int, int action, int) {
            WindowData& data =*reinterpret_cast<WindowData*>(glfwGetWindowUserPointer(window));

            switch (action)
            {
                case GLFW_PRESS:
                {
                    KeyPressedEvent event(static_cast<KeyCode>(key), 0);
                    data.eventCallback(event);
                    break;
                }
                case GLFW_RELEASE:
                {
                    KeyReleasedEvent event(static_cast<KeyCode>(key));
                    data.eventCallback(event);
                    break;
                }
                case GLFW_REPEAT:
                {
                    KeyPressedEvent event(static_cast<KeyCode>(key), 1);
                    data.eventCallback(event);
                    break;
                }
            }
        });

        glfwSetCharCallback(m_WindowHandle, [](GLFWwindow *window, unsigned int keycode) {
            WindowData& data =*reinterpret_cast<WindowData*>(glfwGetWindowUserPointer(window));
            KeyTypedEvent event(static_cast<KeyCode>(keycode));
            data.eventCallback(event);
        });

        glfwSetMouseButtonCallback(m_WindowHandle, [](GLFWwindow *window, int button, int action, int) {
            WindowData& data =*reinterpret_cast<WindowData*>(glfwGetWindowUserPointer(window));

            switch (action)
            {
                case GLFW_PRESS:
                {
                    MouseButtonPressedEvent event(static_cast<MouseCode>(button));
                    data.eventCallback(event);
                    break;
                }
                case GLFW_RELEASE:
                {
                    MouseButtonReleasedEvent event(static_cast<MouseCode>(button));
                    data.eventCallback(event);
                    break;
                }
            }
        });

        glfwSetScrollCallback(m_WindowHandle, [](GLFWwindow *window, double xOffset, double yOffset) {
            WindowData& data = *reinterpret_cast<WindowData*>(glfwGetWindowUserPointer(window));
            
            MouseScrolledEvent event(static_cast<float>(xOffset), static_cast<float>(yOffset));
            data.eventCallback(event);
        });

        glfwSetCursorPosCallback(m_WindowHandle, [](GLFWwindow *window, double xPos, double yPos) {
            WindowData& data = *reinterpret_cast<WindowData*>(glfwGetWindowUserPointer(window));

            MouseMovedEvent event(static_cast<float>(xPos), static_cast<float>(yPos));
            data.eventCallback(event);
        });

        // clang-format on
    }

    GlfwWindowDriver::~GlfwWindowDriver()
    {
        glfwDestroyWindow(m_WindowHandle);
        glfwTerminate(); // TODO: change with glfw context
    }

    void GlfwWindowDriver::OnUpdate()
    {
        glfwPollEvents();
    }

    void GlfwWindowDriver::ToogleFullscreen()
    {
        if (!m_IsFullscreen)
        {
            glfwGetWindowPos(m_WindowHandle, &m_LastPosX, &m_LastPosY);
            glfwGetWindowSize(m_WindowHandle, &m_LastWidth, &m_LastHeight);

            GLFWmonitor *monitor = glfwGetPrimaryMonitor();
            const GLFWvidmode *mode = glfwGetVideoMode(monitor);
            glfwSetWindowMonitor(m_WindowHandle, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);

            m_IsFullscreen = true;
        }
        else
        {
            glfwSetWindowMonitor(m_WindowHandle, nullptr, m_LastPosX, m_LastPosY, m_LastWidth, m_LastHeight, 0);
            m_IsFullscreen = false;
        }
    }

    uint32_t GlfwWindowDriver::GetWidth() const
    {
        return m_Data.width;
    }

    uint32_t GlfwWindowDriver::GetHeight() const
    {
        return m_Data.height;
    }

    void *GlfwWindowDriver::GetNativeWindow() const
    {
        return static_cast<void *>(m_WindowHandle);
    }

    void GlfwWindowDriver::SetEventCallback(const EventCallbackFn &callback)
    {
        m_Data.eventCallback = callback;
    }

    std::vector<const char *> GlfwWindowDriver::GetRequiredVulkanExtensions() const
    {
        uint32_t glfwExtensionCount = 0;
        const char **glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        std::vector<const char *> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

        return extensions;
    }

    VkSurfaceKHR GlfwWindowDriver::GetVulkanSurface(VkInstance instance) const
    {
        VkSurfaceKHR surface;
        VkResult result = glfwCreateWindowSurface(instance, m_WindowHandle, nullptr, &surface);
        CHECK_VK_RESULT(result);

        return surface;
    }

    // Input
    bool GlfwWindowDriver::IsKeyPressed(KeyCode keycode) const
    {
        auto state = glfwGetKey(m_WindowHandle, keycode);
        return state == GLFW_PRESS || state == GLFW_REPEAT;
    }

    bool GlfwWindowDriver::IsMouseButtonPressed(MouseCode button) const
    {
        auto state = glfwGetMouseButton(m_WindowHandle, button);
        return state == GLFW_PRESS;
    }

    std::pair<float, float> GlfwWindowDriver::GetMousePosition() const
    {
        double xPos, yPos;
        glfwGetCursorPos(m_WindowHandle, &xPos, &yPos);
        return {static_cast<float>(xPos), static_cast<float>(yPos)};
    }

    // Cursor
    bool GlfwWindowDriver::IsCursorLocked() const
    {
        return glfwGetInputMode(m_WindowHandle, GLFW_CURSOR) == GLFW_CURSOR_DISABLED;
    }

    void GlfwWindowDriver::SetCursorLocked(bool locked)
    {
        glfwSetInputMode(m_WindowHandle, GLFW_CURSOR, locked ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
    }

} // namespace ve
