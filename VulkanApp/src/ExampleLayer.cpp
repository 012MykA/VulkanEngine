#include "ExampleLayer.hpp"

ExampleLayer::ExampleLayer() : ve::Layer("ExampleLayer")
{
}

ExampleLayer::~ExampleLayer()
{
    m_Renderer->WaitIdle();
}

void ExampleLayer::OnAttach()
{
    auto &window = ve::Application::Get().GetWindow();

    ve::RenderSettings settings{
        .anisotropy = ve::AnisotropicLevel::X16,
        .msaaSamples = ve::MSAASamples::X8,
        .shadowQuality = ve::ShadowQuality::Off,
        .iblIntensity = 1.0f,
    };
    m_Renderer = std::make_unique<ve::RendererPBR>(window, settings);

    m_Camera = std::make_unique<ve::FPSCamera>(ve::FPSCameraDesc{}, window);
    m_Camera->SetPosition({0.0f, 0.0f, 5.0f});

    ve::GLTFLoader loader(*m_Renderer);
    auto sponza = loader.Load("assets/scenes/KhronosGroup glTF-Sample-Models main 2.0-Sponza/glTF/Sponza.gltf");
    m_SceneRenderer.AddScene(sponza);

    SetupScene();
}

void ExampleLayer::OnUpdate(ve::Timestep ts)
{
    m_Camera->OnUpdate(ts);

    const glm::mat4 vp = m_Camera->GetViewProjection();
    ve::CullingResult culling = ve::CullingSystem::Cull(vp, *m_Scene);
    
    m_Renderer->BeginFrame(*m_Camera);
    {
        m_SceneRenderer.Draw(*m_Renderer, *m_Camera);
    
        for (const auto& obj : culling.visibleEntities)
        {
            ve::RenderObject renderObj{
                .transform = obj.GetComponent<ve::TransformComponent>().GetTransform(),
                .mesh = obj.GetComponent<ve::MeshComponent>().mesh,
                .material = obj.GetComponent<ve::MaterialPBRComponent>().material,
                .primitiveIndex = 0,
            };
            m_Renderer->Submit(renderObj);
        }
    }
    m_Renderer->EndFrame();
}

void ExampleLayer::OnEvent(ve::Event &e)
{
    ve::EventDispatcher dp(e);
    dp.Dispatch<ve::WindowResizeEvent>(VE_BIND_EVENT_FN(OnWindowResize));

    m_Camera->OnEvent(e);
}

bool ExampleLayer::OnWindowResize(ve::WindowResizeEvent &e)
{
    m_Camera->SetViewportSize(e.GetWidth(), e.GetHeight());
    m_Renderer->HandleResize(e.GetWidth(), e.GetHeight());

    return false;
}
