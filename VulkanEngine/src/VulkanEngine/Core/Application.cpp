#include "Application.hpp"

namespace VE
{
    Application::Application()
    {
        m_Window = Window::Create();
        m_Window->SetEventCallback(VE_BIND_EVENT_FN(OnEvent));
    }

    void Application::Run()
    {
        while (m_Running)
        {
            m_Window->OnUpdate();
        }
    }

    void Application::OnEvent(Event &e)
    {
        EventDispatcher dp(e);
        dp.Dispatch<WindowCloseEvent>(VE_BIND_EVENT_FN(OnWindowClose));
		dp.Dispatch<WindowResizeEvent>(VE_BIND_EVENT_FN(OnWindowResize));
    }

    bool Application::OnWindowClose(WindowCloseEvent &e)
    {
        m_Running = false;
        return true;
    }

    bool Application::OnWindowResize(WindowResizeEvent &e)
    {
        if (e.GetWidth() == 0 || e.GetHeight() == 0)
        {
            m_Minimized = true;
            return false;
        }
        
        // Renderer::OnWindowResize(e.GetWidth(), e.GetHeight());
        return false;
    }

} // namespace VE
