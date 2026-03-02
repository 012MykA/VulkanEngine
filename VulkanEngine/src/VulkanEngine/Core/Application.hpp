#pragma once

#include "VulkanEngine/Core/Base.hpp"
#include "VulkanEngine/Core/Window.hpp"
#include "VulkanEngine/Core/LayerStack.hpp"
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
        void Close();

        void OnEvent(Event &e);

        void PushLayer(Layer *layer);
        void PushOverlay(Layer *overlay);

    private:
        bool OnWindowClose(WindowCloseEvent &e);
        bool OnWindowResize(WindowResizeEvent &e);

    private:
        Scope<Window> m_Window;
        bool m_Running = true;
        bool m_Minimized = false;
        LayerStack m_LayerStack;
        float m_LastFrameTime = 0.0f;

    private:
        static Application *s_Instance;
    };

    // To be defined in a CLIENT
    Application *CreateApplication();

} // namespace VE
