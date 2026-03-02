#pragma once

#include "VulkanEngine/Core/Base.hpp"
#include "VulkanEngine/Core/Window.hpp"
#include "VulkanEngine/Events/Event.hpp"
#include "VulkanEngine/Events/ApplicationEvent.hpp"

namespace VE
{
    class Application
    {        
    public:
        Application();
        virtual ~Application() {}

    public:
        void Run();

        void OnEvent(Event& e);

    private:
        bool OnWindowClose(WindowCloseEvent& e);
        bool OnWindowResize(WindowResizeEvent& e);

    private:
        Scope<Window> m_Window;
        bool m_Running = true;
        bool m_Minimized = false;
    };

    // To be defined in a CLIENT
    Application *CreateApplication();

} // namespace VE
