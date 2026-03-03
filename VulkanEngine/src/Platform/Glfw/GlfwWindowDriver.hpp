#pragma once

#include "VulkanEngine/Core/Base.hpp"
#if defined(VE_PLATFORM_WINDOWS) || defined(VE_PLATFORM_LINUX)

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

#endif // VE_PLATFORM_WINDOWS || VE_PLATFORM_LINUX
