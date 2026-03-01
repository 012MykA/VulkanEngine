#pragma once

#include "VulkanEngine/Core/Window.hpp"

struct GLFWwindow;

namespace VE
{
    class WindowsWindow : public Window
    {
    public:
        WindowsWindow(const WindowConfig &config);
        ~WindowsWindow();

        virtual void OnUpdate() override;

        virtual uint32_t GetWidth() const override;
        virtual uint32_t GetHeight() const override;

        virtual void SetEventCallback(const EventCallbackFn &callback) override;

    private:
        struct WindowData
        {
            std::string Title;
            uint32_t Width, Height;
            EventCallbackFn EventCallback;
        } m_Data;

        GLFWwindow *m_Window;
    };

} // namespace VE
