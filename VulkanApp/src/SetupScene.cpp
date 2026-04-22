#include "ExampleLayer.hpp"

void ExampleLayer::SetupMeshes()
{
    ve::MeshLoader loader;

    m_CubeMesh = loader.LoadGLTF("assets/models/Cube.glb");
    m_Renderer->UploadMesh(*m_CubeMesh);

    m_SphereMesh = loader.LoadGLTF("assets/models/Sphere.glb");
    m_Renderer->UploadMesh(*m_SphereMesh);

    m_GLTFSceneMesh = loader.LoadGLTF("assets/scenes/KhronosGroup glTF-Sample-Models main 2.0-Sponza/glTF/Sponza.gltf");
    m_Renderer->UploadMesh(*m_GLTFSceneMesh);
}

void ExampleLayer::SetupMaterials()
{
    ve::MaterialLoader loader;

    m_VictorianBrick = loader.LoadFromDirectory("assets/pbr_materials/victorian-brick");
    m_Renderer->UploadMaterial(*m_VictorianBrick);

    m_LightGold = loader.LoadFromDirectory("assets/pbr_materials/light-gold");
    m_Renderer->UploadMaterial(*m_LightGold);

    m_CarbonFiber = loader.LoadFromDirectory("assets/pbr_materials/carbon-fiber");

    auto baseColorTex = ve::TextureLoader().Load("assets/textures/texture.jpg");
    m_Renderer->UploadTexture(*baseColorTex);    
    m_CarbonFiber->SetBaseColorMap(baseColorTex);
    
    m_Renderer->UploadMaterial(*m_CarbonFiber);
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
