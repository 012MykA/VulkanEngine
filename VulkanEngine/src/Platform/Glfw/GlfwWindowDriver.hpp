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

        // Vulkan
        virtual std::vector<const char *> GetRequiredVulkanExtensions() const override;
        virtual VkSurfaceKHR GetVulkanSurface(VkInstance instance) const override;

        // Input
        virtual bool IsKeyPressed(KeyCode keycode) const override;
        virtual bool IsMouseButtonPressed(MouseCode button) const override;
        virtual std::pair<float, float> GetMousePosition() const override;

        // Cursor
        virtual bool IsCursorLocked() const override;
        virtual void SetCursorLocked(bool locked) override;

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
