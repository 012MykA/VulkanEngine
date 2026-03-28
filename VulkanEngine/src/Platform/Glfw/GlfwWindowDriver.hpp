#pragma once

#include "VulkanEngine/Core/Window.hpp"

struct GLFWwindow;

namespace ve
{
    class GlfwWindowDriver : public Window
    {
    public:
        GlfwWindowDriver(const WindowCreateInfo &createInfo);
        ~GlfwWindowDriver() override;

        virtual void OnUpdate() override;

        virtual uint32_t GetWidth() const override;
        virtual uint32_t GetHeight() const override;
        virtual void *GetNativeWindow() const override;

        virtual void SetEventCallback(const EventCallbackFn &callback) override;
        virtual std::vector<const char *> GetRequiredVulkanExtensions() const override;

    private:
        GLFWwindow *m_WindowHandle;

        struct WindowData
        {
            std::string Title;
            uint32_t Width, Height;
            EventCallbackFn EventCallback;
        } m_Data;
    };

} // namespace ve
