#include "VulkanEngine/Core/Application.hpp"
#include "VulkanEngine/Core/Window.hpp"
#include "Instance.hpp"

#include <cassert>

namespace VE
{
    Application *Application::s_Instance = nullptr;

    Application::Application(const std::string &name)
    {
        assert(s_Instance == nullptr && "Application instance alreeady exists!");
        s_Instance = this;

        m_Window = std::make_unique<Window>(WindowConfig(name));
        m_Instance = std::make_unique<Instance>(name, *m_Window);
    }

    Application::~Application()
    {
        s_Instance = nullptr;
    }

    Application &Application::Get()
    {
        assert(s_Instance && "Application instance is null!");
        return *s_Instance;
    }

    void Application::Run()
    {
        while (!m_Window->ShouldClose())
        {
            m_Window->OnUpdate();
        }
    }

}
