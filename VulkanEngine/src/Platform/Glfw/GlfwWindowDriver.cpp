#include "GlfwWindowDriver.hpp"
#include "VulkanEngine/Core/Log.hpp"
#include "VulkanEngine/Events/ApplicationEvent.hpp"
#include "VulkanEngine/Events/KeyEvent.hpp"
#include "VulkanEngine/Events/MouseEvent.hpp"
#include "VulkanEngine/Vulkan/Debug/VulkanValidation.hpp"

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

    GlfwWindowDriver::GlfwWindowDriver(const WindowCreateInfo &createInfo)
    {
        m_Data.Title = createInfo.Title;
        m_Data.Width = createInfo.Width;
        m_Data.Height = createInfo.Height;

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
        glfwWindowHint(GLFW_RESIZABLE, createInfo.Resizable ? GLFW_TRUE : GLFW_FALSE);

        // Handle Fullscreen
        GLFWmonitor *const monitor = createInfo.Fullscreen ? glfwGetPrimaryMonitor() : nullptr;
        m_WindowHandle = glfwCreateWindow(createInfo.Width, createInfo.Height, createInfo.Title.c_str(), monitor, nullptr);
        if (!m_WindowHandle)
            throw std::runtime_error("failed to create GLFW window!");

        // Set Window Icon
        if (!createInfo.IconPath.empty())
        {
            GLFWimage icon;
            icon.pixels = stbi_load(createInfo.IconPath.string().c_str(), &icon.width, &icon.height, nullptr, STBI_rgb_alpha);

            if (icon.pixels == nullptr)
            {
                VE_CORE_ERROR("failed to load window icon: {0}", createInfo.IconPath);
            }
            else
            {
                glfwSetWindowIcon(m_WindowHandle, 1, &icon);
                stbi_image_free(icon.pixels);
            }
        }

        // User pointer
        glfwSetWindowUserPointer(m_WindowHandle, &m_Data);

        // Set GLFW callbacks
        glfwSetWindowSizeCallback(m_WindowHandle, [](GLFWwindow *window, int width, int height)
                                  {
            WindowData& data = *reinterpret_cast<WindowData*>(glfwGetWindowUserPointer(window));
            data.Width = static_cast<uint32_t>(width);
            data.Height = static_cast<uint32_t>(height);

            WindowResizeEvent event(width, height);
            data.EventCallback(event); });

        glfwSetWindowCloseCallback(m_WindowHandle, [](GLFWwindow *window)
                                   {
            WindowData& data =*reinterpret_cast<WindowData*>(glfwGetWindowUserPointer(window));

            WindowCloseEvent event;
            data.EventCallback(event); });

        glfwSetKeyCallback(m_WindowHandle, [](GLFWwindow *window, int key, int, int action, int)
                           {
            WindowData& data =*reinterpret_cast<WindowData*>(glfwGetWindowUserPointer(window));

            switch (action)
            {
                case GLFW_PRESS:
                {
                    KeyPressedEvent event(static_cast<KeyCode>(key), 0);
                    data.EventCallback(event);
                    break;
                }
                case GLFW_RELEASE:
                {
                    KeyReleasedEvent event(static_cast<KeyCode>(key));
                    data.EventCallback(event);
                    break;
                }
                case GLFW_REPEAT:
                {
                    KeyPressedEvent event(static_cast<KeyCode>(key), 1);
                    data.EventCallback(event);
                    break;
                }
            } });

        glfwSetCharCallback(m_WindowHandle, [](GLFWwindow *window, unsigned int keycode)
                            {
            WindowData& data =*reinterpret_cast<WindowData*>(glfwGetWindowUserPointer(window));
            KeyTypedEvent event(static_cast<KeyCode>(keycode));
            data.EventCallback(event); });

        glfwSetMouseButtonCallback(m_WindowHandle, [](GLFWwindow *window, int button, int action, int)
                                   {
            WindowData& data =*reinterpret_cast<WindowData*>(glfwGetWindowUserPointer(window));

            switch (action)
            {
                case GLFW_PRESS:
                {
                    MouseButtonPressedEvent event(static_cast<MouseCode>(button));
                    data.EventCallback(event);
                    break;
                }
                case GLFW_RELEASE:
                {
                    MouseButtonReleasedEvent event(static_cast<MouseCode>(button));
                    data.EventCallback(event);
                    break;
                }
            } });

        glfwSetScrollCallback(m_WindowHandle, [](GLFWwindow *window, double xOffset, double yOffset)
                              {
            WindowData& data = *reinterpret_cast<WindowData*>(glfwGetWindowUserPointer(window));
            
            MouseScrolledEvent event(static_cast<float>(xOffset), static_cast<float>(yOffset));
            data.EventCallback(event); });

        glfwSetCursorPosCallback(m_WindowHandle, [](GLFWwindow *window, double xPos, double yPos)
                                 {
            WindowData& data = *reinterpret_cast<WindowData*>(glfwGetWindowUserPointer(window));

            MouseMovedEvent event(static_cast<float>(xPos), static_cast<float>(yPos));
            data.EventCallback(event); });
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

    uint32_t GlfwWindowDriver::GetWidth() const
    {
        return m_Data.Width;
    }

    uint32_t GlfwWindowDriver::GetHeight() const
    {
        return m_Data.Height;
    }

    void *GlfwWindowDriver::GetNativeWindow() const
    {
        return static_cast<void *>(m_WindowHandle);
    }

    void GlfwWindowDriver::SetEventCallback(const EventCallbackFn &callback)
    {
        m_Data.EventCallback = callback;
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
