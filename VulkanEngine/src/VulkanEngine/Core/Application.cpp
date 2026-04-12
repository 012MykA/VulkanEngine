#include "Application.hpp"
#include "VulkanEngine/Utils/PlatformUtils.hpp"
#include "VulkanEngine/Core/Timestep.hpp"
#include "VulkanEngine/Core/Log.hpp"

#include <cassert>

namespace ve
{
    Application *Application::s_Instance = nullptr;

    Application::Application(const ApplicationDesc &desc) : m_Desc(desc)
    {
        assert(!s_Instance && "Application already exists!");
        s_Instance = this;

        std::filesystem::path workingDir(m_Desc.workingDirectory);
        if (std::filesystem::exists(workingDir))
        {
            std::filesystem::current_path(workingDir);
        }

        m_Window = Window::Create(m_Desc.windowDesc);
        m_Window->SetEventCallback(VE_BIND_EVENT_FN(OnEvent));
    }

    void Application::Run()
    {
        while (m_Running)
        {
            float time = Time::GetTime();
            Timestep ts = time - m_LastFrameTime;
            m_LastFrameTime = time;

            if (!m_Minimized)
            {
                for (Layer *layer : m_LayerStack)
                {
                    layer->OnUpdate(ts);
                }
            }

            m_Window->OnUpdate();
        }
    }

    void Application::Close()
    {
        m_Running = false;
    }

    void Application::OnEvent(Event &e)
    {
        EventDispatcher dp(e);
        dp.Dispatch<WindowCloseEvent>(VE_BIND_EVENT_FN(OnWindowClose));
        dp.Dispatch<WindowResizeEvent>(VE_BIND_EVENT_FN(OnWindowResize));

        for (auto it = m_LayerStack.rbegin(); it != m_LayerStack.rend(); it++)
        {
            if (e.Handled)
                break;
            (*it)->OnEvent(e);
        }
    }

    void Application::PushLayer(Layer *layer)
    {
        m_LayerStack.PushLayer(layer);
    }

    void Application::PushOverlay(Layer *overlay)
    {
        m_LayerStack.PushOverlay(overlay);
    }

    bool Application::OnWindowClose(WindowCloseEvent &)
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
        m_Minimized = false;

        return false;
    }

} // namespace ve
