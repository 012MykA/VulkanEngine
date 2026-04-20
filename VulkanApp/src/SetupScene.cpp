#include "ExampleLayer.hpp"

void ExampleLayer::SetupMeshes()
{
    ve::MeshLoader loader;
    
    m_CubeMesh = loader.LoadGLB("assets/models/Cube.glb");
    m_Renderer->UploadMesh(*m_CubeMesh);

    m_SphereMesh = loader.LoadGLB("assets/models/Sphere.glb");
    m_Renderer->UploadMesh(*m_SphereMesh);

    m_GLTFSceneMesh = loader.LoadGLB("assets/scenes/KhronosGroup glTF-Sample-Models main 2.0-Sponza/glTF/Sponza.gltf");
    m_Renderer->UploadMesh(*m_GLTFSceneMesh);
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

    {
        auto entity = m_Scene->CreateEntity("Sponza");
        entity.AddComponent<ve::TransformComponent>().SetScale(glm::vec3(0.008f));
        entity.AddComponent<ve::MeshComponent>(m_GLTFSceneMesh);
        entity.AddComponent<ve::MaterialPBRComponent>(m_CarbonFiber);
    }
}
