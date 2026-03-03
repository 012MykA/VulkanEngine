#pragma once

#include "VulkanEngine/Core/Base.hpp"
#ifdef VE_PLATFORM_WINDOWS

#include "VulkanEngine/Core/Window.hpp"

struct GLFWwindow;

namespace ve
{
    class WindowsWindowDriver : public Window
    {
    public:
        WindowsWindowDriver(const WindowCreateInfo &createInfo);
        ~WindowsWindowDriver() override;

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

} // namespace ve

#endif // VE_PLATFORM_WINDOWS
