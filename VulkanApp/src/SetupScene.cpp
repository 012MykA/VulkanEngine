#include "ExampleLayer.hpp"

void ExampleLayer::SetupMeshes()
{
    ve::MeshLoader loader;
    
    m_CubeMesh = loader.LoadGLB("assets/models/Cube.glb");
    m_Renderer->UploadMesh(*m_CubeMesh);

    m_SphereMesh = loader.LoadGLB("assets/models/Sphere.glb");
    m_Renderer->UploadMesh(*m_SphereMesh);
}

void ExampleLayer::SetupMaterials()
{
    m_VictorianBrick = ve::MaterialPBR::Load(
        "assets/pbr_materials/victorian-brick",
        m_Renderer->GetAllocator(),
        m_Renderer->GetLogicalDevice(),
        m_Renderer->GetGraphicsImmediateSubmit());
    m_Renderer->BuildMaterial(*m_VictorianBrick);

    m_LightGold = ve::MaterialPBR::Load(
        "assets/pbr_materials/light-gold",
        m_Renderer->GetAllocator(),
        m_Renderer->GetLogicalDevice(),
        m_Renderer->GetGraphicsImmediateSubmit());
    m_Renderer->BuildMaterial(*m_LightGold);

    m_CarbonFiber = ve::MaterialPBR::Load(
        "assets/pbr_materials/carbon-fiber",
        m_Renderer->GetAllocator(),
        m_Renderer->GetLogicalDevice(),
        m_Renderer->GetGraphicsImmediateSubmit());
    m_Renderer->BuildMaterial(*m_CarbonFiber);
}

void ExampleLayer::SetupObjects()
{
    {
        auto entity = m_Scene->CreateEntity();
        entity.AddComponent<ve::TransformComponent>().SetPosition({-3.0f, 0.0f, 0.0f});
        entity.AddComponent<ve::MeshComponent>(m_SphereMesh);
        entity.AddComponent<ve::MaterialPBRComponent>(m_LightGold);
    }

    {
        auto entity = m_Scene->CreateEntity();
        entity.AddComponent<ve::TransformComponent>().SetPosition({0.0f, 0.0f, 0.0f});
        entity.AddComponent<ve::MeshComponent>(m_SphereMesh);
        entity.AddComponent<ve::MaterialPBRComponent>(m_VictorianBrick);
    }

    {
        auto entity = m_Scene->CreateEntity();
        entity.AddComponent<ve::TransformComponent>().SetPosition({3.0f, 0.0f, 0.0f});
        entity.AddComponent<ve::MeshComponent>(m_SphereMesh);
        entity.AddComponent<ve::MaterialPBRComponent>(m_CarbonFiber);
    }

    // Lights
    m_Renderer->AddLight({-1.5f, 1.5f, 1.5f}, {0.0f, 0.9f, 0.4f}, 2.0f);
    m_Renderer->AddLight({1.5f, 1.5f, -1.5f}, {0.9f, 0.2f, 0.8f}, 2.0f);
}
