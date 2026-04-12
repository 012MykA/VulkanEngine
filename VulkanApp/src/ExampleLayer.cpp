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
    m_Camera = std::make_unique<ve::Camera>(window, ve::CameraDesc{});

    SetupObjects();
}

void ExampleLayer::OnUpdate(ve::Timestep ts)
{
    m_Camera->OnUpdate(ts);

    m_Renderer->BeginFrame(*m_Camera);
    for (const auto &obj : m_Objects)
    {
        m_Renderer->Submit(obj);
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

    auto sloppyMortarStoneWall = ve::MaterialPBR::Load(
        "assets/pbr_materials/sloppy-mortar-stone-wall",
        m_Renderer->GetAllocator(),
        m_Renderer->GetLogicalDevice(),
        m_Renderer->GetGraphicsImmediateSubmit());
    m_Renderer->BuildMaterial(*sloppyMortarStoneWall);

    // Objects
    m_Objects.push_back(ve::RenderObject{
        .mesh = sphereMesh,
        .material = lightGold,
        .transform = glm::translate(glm::mat4(1.0f), {-3.0f, 0.0f, 0.0f}),
    });

    m_Objects.push_back(ve::RenderObject{
        .mesh = cubeMesh,
        .material = victorianBrick,
        .transform = glm::translate(glm::mat4(1.0f), {0.0f, 0.0f, 0.0f}),
    });

    m_Objects.push_back(ve::RenderObject{
        .mesh = cubeMesh,
        .material = sloppyMortarStoneWall,
        .transform = glm::translate(glm::mat4(1.0f), {3.0f, 0.0f, 0.0f}),
    });

    // Lights
    m_Renderer->AddLight({-1.5f, 1.5f, 1.5f}, glm::vec3(1.0f), 1.0f);
    m_Renderer->AddLight({1.5f, 1.5f, -1.5f}, glm::vec3(1.0f), 1.0f);
}

bool ExampleLayer::OnWindowResize(ve::WindowResizeEvent &e)
{
    m_Camera->OnResize(static_cast<float>(e.GetWidth()), static_cast<float>(e.GetHeight()));

    m_Renderer->HandleResize(e.GetWidth(), e.GetHeight());

    return false;
}
