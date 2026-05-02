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
    m_Camera->SetPosition({-8.0f, 1.0f, 0.0f});
    m_Camera->SetYaw(0);

    BuildLightTestScene();
}

void ExampleLayer::OnUpdate(ve::Timestep ts)
{
    m_Camera->OnUpdate(ts);

    m_Renderer->BeginFrame(*m_Camera);
    {
        m_SceneRenderer.Draw(*m_Renderer, *m_Camera);
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
