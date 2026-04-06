#include "AppWindow.hpp"
#include "VulkanEngine/Core/Base.hpp"
#include "VulkanEngine/Renderer/Renderer.hpp"
#include "VulkanEngine/Core/Log.hpp"

#include "VulkanEngine/Events/Event.hpp"

namespace ve
{
    AppWindow::AppWindow(const WindowDesc &desc, CloseCallback onClose)
        : m_Title(desc.title), m_OnClose(std::move(onClose))
    {
        m_Window = Window::Create(desc);
        m_Window->SetEventCallback(VE_BIND_EVENT_FN(AppWindow::OnEvent));

        m_Renderer = std::make_unique<Renderer>(*m_Window);

        VE_CORE_TRACE("AppWindow '{}' created ({}x{})", m_Title, desc.width, desc.height);
    }

    AppWindow::~AppWindow()
    {
        m_Renderer->WaitIdle();
        VE_CORE_TRACE("AppWindow '{}' destroyed", m_Title);
    }

    void AppWindow::OnUpdate(Timestep ts)
    {
        if (m_Minimized)
            return;

        for (Layer *layer : m_LayerStack)
            layer->OnUpdate(ts);
    }

    void AppWindow::OnEvent(Event &e)
    {
        EventDispatcher dp(e);
        dp.Dispatch<WindowCloseEvent>(VE_BIND_EVENT_FN(AppWindow::OnWindowClose));
        dp.Dispatch<WindowResizeEvent>(VE_BIND_EVENT_FN(AppWindow::OnWindowResize));

        for (auto it = m_LayerStack.rbegin(); it != m_LayerStack.rend(); ++it)
        {
            if (e.Handled)
                break;

            (*it)->OnEvent(e);
        }
    }

    void AppWindow::PushLayer(Layer *layer)
    {
        m_LayerStack.PushLayer(layer);
    }

    void AppWindow::PushOverlay(Layer *overlay)
    {
        m_LayerStack.PushOverlay(overlay);
    }

    void AppWindow::RenderFrame()
    {
        if (m_Minimized)
            return;
    }

    bool AppWindow::OnWindowClose(WindowCloseEvent &e)
    {
        m_Open = false;
        if (m_OnClose)
            m_OnClose(this);

        return true;
    }

    bool AppWindow::OnWindowResize(WindowResizeEvent &e)
    {
        if (e.GetWidth() == 0 || e.GetHeight() == 0)
        {
            m_Minimized = true;
            return false;
        }
        m_Minimized = false;

        // OnResize calls

        m_Renderer->HandleResize(e.GetWidth(), e.GetHeight());

        // ---

        return false;
    }

} // namespace ve
