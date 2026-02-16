#include "Application.hpp"

namespace VE
{
    Application::Application(const WindowConfig &config)
    {
        m_Window = std::make_unique<Window>(config);
    }

    void Application::Run()
    {
        while (!m_Window->ShouldClose())
        {
            m_Window->OnUpdate();
        }
    }

}
