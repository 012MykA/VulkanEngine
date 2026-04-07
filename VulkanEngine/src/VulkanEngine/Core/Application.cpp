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

        m_Renderer = std::make_unique<Renderer>(*m_Window);

        m_Camera = std::make_unique<Camera>(*m_Window, CameraDesc{});

        // TODO: remove
        std::vector<Vertex> vertices = {
            {{-0.5f, -0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
            {{0.5f, -0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
            {{0.5f, 0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}},
            {{-0.5f, 0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},

            {{0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {-1.0f, 0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
            {{-0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {-1.0f, 0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
            {{-0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {-1.0f, 0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}},
            {{0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {-1.0f, 0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},

            {{-0.5f, -0.5f, -0.5f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f, 1.0f}, {0.0f, 1.0f}},
            {{-0.5f, -0.5f, 0.5f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f, 1.0f}, {1.0f, 1.0f}},
            {{-0.5f, 0.5f, 0.5f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f, 1.0f}, {1.0f, 0.0f}},
            {{-0.5f, 0.5f, -0.5f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f, 1.0f}, {0.0f, 0.0f}},

            {{0.5f, -0.5f, 0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, -1.0f, 1.0f}, {0.0f, 1.0f}},
            {{0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, -1.0f, 1.0f}, {1.0f, 1.0f}},
            {{0.5f, 0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, -1.0f, 1.0f}, {1.0f, 0.0f}},
            {{0.5f, 0.5f, 0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, -1.0f, 1.0f}, {0.0f, 0.0f}},

            {{-0.5f, 0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
            {{0.5f, 0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
            {{0.5f, 0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}},
            {{-0.5f, 0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},

            {{-0.5f, -0.5f, -0.5f}, {0.0f, -1.0f, 0.0f}, {1.0f, 0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
            {{0.5f, -0.5f, -0.5f}, {0.0f, -1.0f, 0.0f}, {1.0f, 0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
            {{0.5f, -0.5f, 0.5f}, {0.0f, -1.0f, 0.0f}, {1.0f, 0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}},
            {{-0.5f, -0.5f, 0.5f}, {0.0f, -1.0f, 0.0f}, {1.0f, 0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
        };

        std::vector<uint32_t> indices = {
            0, 1, 2, 2, 3, 0,
            4, 5, 6, 6, 7, 4,
            8, 9, 10, 10, 11, 8,
            12, 13, 14, 14, 15, 12,
            16, 17, 18, 18, 19, 16,
            20, 21, 22, 22, 23, 20};

        auto cubeMesh = std::make_shared<Mesh>();
        cubeMesh->SetVertices(vertices);
        cubeMesh->SetIndices(indices);
        cubeMesh->SetName("Cube");
        m_Renderer->UploadMesh(*cubeMesh);

        // Textures
        auto basicTexture = m_Renderer->LoadTexture(
            "assets/textures/texture.jpg",
            TextureDesc{
                .generateMips = false,
            });

        auto mipmappedTexture = m_Renderer->LoadTexture(
            "assets/textures/texture.jpg",
            TextureDesc{
                .generateMips = true,
            });

        // Materials
        auto defaultMaterial = std::make_shared<Material>();
        defaultMaterial->SetBaseColorTexture(basicTexture);
        defaultMaterial->SetName("Default");
        m_Renderer->BuildMaterial(*defaultMaterial);

        auto mipmappedMaterial = std::make_shared<Material>();
        mipmappedMaterial->SetBaseColorTexture(mipmappedTexture);
        mipmappedMaterial->SetName("Mipmapped");
        m_Renderer->BuildMaterial(*mipmappedMaterial);

        // Objects
        m_Objects.push_back(RenderObject{
            .mesh = cubeMesh,
            .material = defaultMaterial,
            .model = glm::translate(glm::mat4(1.0f), {-1.0f, 0.0f, -1.0f}),
        });

        m_Objects.push_back(RenderObject{
            .mesh = cubeMesh,
            .material = defaultMaterial,
            .model = glm::translate(glm::mat4(1.0f), {-0.5f, 0.0f, -20.0f}),
        });

        // mipmapped
        m_Objects.push_back(RenderObject{
            .mesh = cubeMesh,
            .material = mipmappedMaterial,
            .model = glm::translate(glm::mat4(1.0f), {1.0f, 0.0f, -1.0f}),
        });

        m_Objects.push_back(RenderObject{
            .mesh = cubeMesh,
            .material = mipmappedMaterial,
            .model = glm::translate(glm::mat4(1.0f), {0.5f, 0.0f, -20.0f}),
        });

        VE_CORE_INFO("Objects count: {}", m_Objects.size());

        m_Renderer->SetLight({-1.0f, -1.0f, -1.0f});
        // ---
    }

    Application::~Application()
    {
        m_Renderer->WaitIdle();
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

                m_Camera->OnUpdate(ts);

                m_Renderer->BeginFrame(*m_Camera);

                static float rot = 0.0f;
                float speed = 50.0f;

                for (const auto &obj : m_Objects)
                {
                    // glm::mat4 transform = glm::rotate(obj.model, glm::radians(rot), glm::vec3(0.0f, 1.0f, 0.0f));

                    m_Renderer->Submit(*obj.mesh, *obj.material, obj.model);
                }

                rot += speed * ts;

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
