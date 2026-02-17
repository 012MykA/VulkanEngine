#pragma once

#include <string>
#include <memory>
#include <vector>

namespace VE
{
    class Window;
    class Instance;
    class DebugUtilsMessenger;
    class Surface;
    class Renderer;

    class Application
    {
    public:
        Application(const std::string &name);
        virtual ~Application();

        static Application &Get();

        void Run();

    private:
        std::unique_ptr<Window> m_Window;
        std::unique_ptr<Instance> m_Instance;
        std::unique_ptr<DebugUtilsMessenger> m_DebugUtilsMessenger;
        std::unique_ptr<Surface> m_Surface;

        std::unique_ptr<Renderer> m_Renderer;

    private:
        static Application *s_Instance;
    };

}
