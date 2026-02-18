#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <functional>

struct GLFWwindow;

namespace VE
{
    struct WindowConfig
    {
        std::string Title = "Vulkan Window";
        uint32_t Width = 800, Height = 600;
        bool Resizable = true;
        bool Fullscreen = false;
    };

    class Window final
    {
    public:
        explicit Window(const WindowConfig &config);
        ~Window();

        GLFWwindow *Handle() const { return m_Window; }

        std::vector<const char *> GetRequiredInstanceExtensions() const;

        void OnUpdate();

        std::function<void(int width, int height)> OnResize;

        bool IsMinimized() const;
        void WaitForEvents() const;
        bool ShouldClose() const;

    private:
        WindowConfig m_Config;
        GLFWwindow *m_Window;
    };

}
