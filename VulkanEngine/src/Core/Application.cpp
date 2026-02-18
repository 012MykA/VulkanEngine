#include "Application.hpp"
#include "Window.hpp"
#include "Instance.hpp"
#include "DebugUtilsMessenger.hpp"
#include "Surface.hpp"
#include "Renderer.hpp"

#include <cassert>

namespace VE
{
    Application *Application::s_Instance = nullptr;

#ifdef NDEBUG
    const bool enableValidationLayers = false;
#else
    const bool enableValidationLayers = true;
#endif

    Application::Application(const std::string &name)
    {
        assert(s_Instance == nullptr && "Application instance alreeady exists!");
        s_Instance = this;

        const auto validationLayers = enableValidationLayers
                                          ? std::vector<const char *>{"VK_LAYER_KHRONOS_validation"}
                                          : std::vector<const char *>();

        m_Window = std::make_unique<Window>(WindowConfig(name));
        m_Instance = std::make_unique<Instance>(name, *m_Window, validationLayers);
        m_DebugUtilsMessenger = std::make_unique<DebugUtilsMessenger>(*m_Instance);
        m_Surface = std::make_unique<Surface>(*m_Instance, *m_Window);
        m_Renderer = std::make_unique<Renderer>(*m_Instance, *m_Surface, *m_Window);
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
            m_Renderer->DrawFrame();
        }
    }

}
