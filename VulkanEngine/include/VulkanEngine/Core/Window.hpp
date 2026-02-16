#pragma once

#include <string>
#include <cstdint>

struct GLFWwindow;

namespace VE
{
    struct WindowConfig
    {
        std::string Title = "Vulkan Window";
        uint32_t Width = 800, Height = 600;
        bool Resizable = false;
        bool Fullscreen = false;
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
