#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <cstdint>
#include <string>
#include <functional>

namespace VE
{
    struct WindowConfig
    {
        std::string Title;
        uint32_t Width, Height;
        bool Resizable;
        bool Fullscreen;
    };

    class Window final
    {
    public:
        explicit Window(const WindowConfig &config);
        ~Window();

        void OnUpdate();
        bool ShouldClose() const;

    private:
        WindowConfig m_Config;
        GLFWwindow *m_Window;
    };

}
