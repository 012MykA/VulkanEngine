#include "Application.hpp"
#include "VulkanEngine/Core/Timestep.hpp"
#include "VulkanEngine/Utils/PlatformUtils.hpp"
#include "Generation.hpp"

#include <cassert>

namespace ve
{
    Application *Application::s_Instance = nullptr;

    Application::Application(const ApplicationCreateInfo &createInfo) : m_Info(createInfo)
    {
        assert(!s_Instance && "Application already exists!");
        s_Instance = this;

        std::filesystem::path workingDir(createInfo.WorkingDirectory);
        if (std::filesystem::exists(workingDir))
        {
            std::filesystem::current_path(workingDir);
        }

        m_Window = Window::Create(createInfo.WindowInfo);
        m_Window->SetEventCallback(VE_BIND_EVENT_FN(OnEvent));

        m_Renderer = std::make_unique<Renderer>(*m_Window);

        m_Camera = std::make_unique<Camera>(*m_Window, CameraDesc{});

        // TODO: remove
        auto terrainMesh = GenerateTerrain(1000, 0.5f);

        m_Renderer->UploadMesh(*terrainMesh);

        auto terrainMaterial = std::make_shared<Material>();
        terrainMaterial->SetName("TerrainMaterial");
        m_Renderer->BuildMaterial(*terrainMaterial);

        RenderObject terrain;
        terrain.mesh = terrainMesh;
        terrain.material = terrainMaterial;
        terrain.transform = glm::translate(glm::mat4(1.0f), {-250.0f, -100.0f, -500.0f}); 

        m_Objects.push_back(terrain);
    }

    void Application::Run()
    {
        while (m_Running)
        {
            float time = Time::GetTime();
            Timestep timestep = time - m_LastFrameTime;
            m_LastFrameTime = time;

            if (!m_Minimized)
            {
                for (Layer *layer : m_LayerStack)
                {
                    layer->OnUpdate(timestep);
                }

                m_Camera->OnUpdate(timestep);

                m_Renderer->BeginFrame(*m_Camera);

                for (const auto &obj : m_Objects)
                {
                    m_Renderer->Submit(*obj.mesh, *obj.material, obj.transform);
                }

                m_Renderer->EndFrame();
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

        m_Camera->OnEvent(e);

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

        m_Camera->OnResize(static_cast<float>(e.GetWidth()), static_cast<float>(e.GetHeight()));
        m_Renderer->HandleResize(e.GetWidth(), e.GetHeight());

        return false;
    }

} // namespace ve
