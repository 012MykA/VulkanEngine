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

    m_Renderer = std::make_unique<ve::RendererPBR>(window);
    m_Camera = std::make_unique<ve::FPSCamera>(ve::FPSCameraDesc{}, window);
    m_Camera->SetPosition({0.0f, 0.0f, 5.0f});

    m_Scene = std::make_unique<ve::Scene>();

    SetupObjects();
}

void ExampleLayer::OnUpdate(ve::Timestep ts)
{
    m_Camera->OnUpdate(ts);

    const glm::mat4 vp = m_Camera->GetViewProjection();
    ve::CullingResult culling = ve::CullingSystem::Cull(vp, *m_Scene);

    static ve::CullingResult previousCulling{};
    if (previousCulling.totalTested != culling.totalTested || previousCulling.totalCulled != culling.totalCulled)
    {
        VE_TRACE("Visible: {}/{}", culling.totalTested - culling.totalCulled, culling.totalTested);
        previousCulling = culling;
    }

    m_Renderer->BeginFrame(*m_Camera);
    for (const auto &obj : culling.visibleEntities)
    {
        ve::RenderObject renderObj{
            .transform = obj.GetComponent<ve::TransformComponent>().GetTransform(),
            .mesh = obj.GetComponent<ve::MeshComponent>().mesh,
            .material = obj.GetComponent<ve::MaterialPBRComponent>().material,
        };

        m_Renderer->Submit(renderObj);
    }
    m_Renderer->EndFrame();
}

void ExampleLayer::OnEvent(ve::Event &e)
{
    ve::EventDispatcher dp(e);
    dp.Dispatch<ve::WindowResizeEvent>(VE_BIND_EVENT_FN(OnWindowResize));

    m_Camera->OnEvent(e);
}

void ExampleLayer::SetupObjects()
{
    // Meshes
    auto cubeMesh = ve::Mesh::Load("assets/models/Cube.glb");
    m_Renderer->UploadMesh(*cubeMesh);

    auto sphereMesh = ve::Mesh::Load("assets/models/Sphere.glb");
    m_Renderer->UploadMesh(*sphereMesh);

    // Materials
    auto victorianBrick = ve::MaterialPBR::Load(
        "assets/pbr_materials/victorian-brick",
        m_Renderer->GetAllocator(),
        m_Renderer->GetLogicalDevice(),
        m_Renderer->GetGraphicsImmediateSubmit());
    m_Renderer->BuildMaterial(*victorianBrick);

    auto lightGold = ve::MaterialPBR::Load(
        "assets/pbr_materials/light-gold",
        m_Renderer->GetAllocator(),
        m_Renderer->GetLogicalDevice(),
        m_Renderer->GetGraphicsImmediateSubmit());
    m_Renderer->BuildMaterial(*lightGold);

    auto carbonFiber = ve::MaterialPBR::Load(
        "assets/pbr_materials/carbon-fiber",
        m_Renderer->GetAllocator(),
        m_Renderer->GetLogicalDevice(),
        m_Renderer->GetGraphicsImmediateSubmit());
    m_Renderer->BuildMaterial(*carbonFiber);

    // Objects
    {
        auto entity = m_Scene->CreateEntity();
        entity.AddComponent<ve::TransformComponent>().SetPosition({-3.0f, 0.0f, 0.0f});
        entity.AddComponent<ve::MeshComponent>(sphereMesh);
        entity.AddComponent<ve::MaterialPBRComponent>(lightGold);
    }

    {
        auto entity = m_Scene->CreateEntity();
        entity.AddComponent<ve::TransformComponent>().SetPosition({0.0f, 0.0f, 0.0f});
        entity.AddComponent<ve::MeshComponent>(sphereMesh);
        entity.AddComponent<ve::MaterialPBRComponent>(victorianBrick);
    }

    {
        auto entity = m_Scene->CreateEntity();
        entity.AddComponent<ve::TransformComponent>().SetPosition({3.0f, 0.0f, 0.0f});
        entity.AddComponent<ve::MeshComponent>(sphereMesh);
        entity.AddComponent<ve::MaterialPBRComponent>(carbonFiber);
    }

    // Lights
    m_Renderer->AddLight({-1.5f, 1.5f, 1.5f}, glm::vec3(1.0f), 1.0f);
    m_Renderer->AddLight({1.5f, 1.5f, -1.5f}, glm::vec3(0.9f, 0.2f, 0.8f), 2.0f);
}

bool ExampleLayer::OnWindowResize(ve::WindowResizeEvent &e)
{
    m_Camera->SetViewportSize(e.GetWidth(), e.GetHeight());
    m_Renderer->HandleResize(e.GetWidth(), e.GetHeight());

    return false;
}
