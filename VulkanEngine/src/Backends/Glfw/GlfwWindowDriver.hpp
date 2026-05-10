#pragma once

#include "VulkanEngine/Core/Window.hpp"

struct GLFWwindow;

namespace ve
{
    class GlfwWindowDriver : public Window
    {
    public:
        GlfwWindowDriver(const WindowDesc &desc);
        ~GlfwWindowDriver() override;

        virtual void OnUpdate() override;

        virtual void ToogleFullscreen() override;

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

        // Getters
        virtual uint32_t GetWidth() const override;
        virtual uint32_t GetHeight() const override;
        virtual void *GetNativeWindow() const override;

        // Setters
        virtual void SetEventCallback(const EventCallbackFn &callback) override;

    private:
        GLFWwindow *m_WindowHandle;

        bool m_IsFullscreen;
        int m_LastPosX, m_LastPosY;
        int m_LastWidth, m_LastHeight;

        struct WindowData
        {
            std::string title;
            uint32_t width, height;
            EventCallbackFn eventCallback;
        } m_Data;
    };

} // namespace ve
