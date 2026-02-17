#pragma once

#include <string>
#include <memory>

namespace VE
{
    class Window;
    class Instance;
    class DebugUtilsMessenger;
    class Surface;
    class Device;

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
        std::unique_ptr<Device> m_Device;

    private:
        static Application *s_Instance;
    };

}
