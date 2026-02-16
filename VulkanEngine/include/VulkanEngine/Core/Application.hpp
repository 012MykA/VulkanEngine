#pragma once

#include <string>
#include <memory>

namespace VE
{
    class Window;
    class Instance;
    class DebugUtilsMessenger;
    
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

    private:
        static Application *s_Instance;
    };

}
