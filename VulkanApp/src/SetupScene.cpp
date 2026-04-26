#include "ExampleLayer.hpp"

void ExampleLayer::SetupScene()
{
    SetupMeshes();
    SetupMaterials();
    SetupObjects();

    // m_Renderer->SetSkybox("assets/skyboxes/street/faces/");
    // m_Renderer->SetSkyboxEnabled(true);
}

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
    m_DefaultMaterial = std::make_shared<ve::MaterialPBR>();
    m_DefaultMaterial->SetName("Default");
    m_Renderer->UploadMaterial(*m_DefaultMaterial);

    m_PurpleNeon = std::make_shared<ve::MaterialPBR>();
    m_PurpleNeon->SetName("Purple neon");
    m_PurpleNeon->SetEmissiveColorFactor({0.9f, 0.0f, 0.8f});
    m_PurpleNeon->SetEmissiveStrength(5.0f);
    m_Renderer->UploadMaterial(*m_PurpleNeon);

    m_BlueNeon = std::make_shared<ve::MaterialPBR>();
    m_BlueNeon->SetName("Blue neon");
    m_BlueNeon->SetEmissiveColorFactor({0.0f, 0.7f, 1.0f});
    m_BlueNeon->SetEmissiveStrength(5.0f);
    m_Renderer->UploadMaterial(*m_BlueNeon);
}

void ExampleLayer::SetupObjects()
{
    {
        glm::vec3 position = {-3.0f, 1.0f, 0.0f};

        auto entity = m_Scene->CreateEntity();
        entity.AddComponent<ve::TransformComponent>().SetPosition(position);
        entity.AddComponent<ve::MeshComponent>(m_SphereMesh);
        entity.AddComponent<ve::MaterialPBRComponent>(m_PurpleNeon);

        m_Renderer->AddLight(position,
                             m_PurpleNeon->GetEmissiveColorFactor(),
                             m_PurpleNeon->GetEmissiveStrength());
    }

    {
        auto entity = m_Scene->CreateEntity();
        entity.AddComponent<ve::TransformComponent>().SetPosition({0.0f, 1.0f, 0.0f});
        entity.AddComponent<ve::MeshComponent>(m_SphereMesh);
        entity.AddComponent<ve::MaterialPBRComponent>(m_DefaultMaterial);
    }

    {
        glm::vec3 position = {3.0f, 1.0f, 0.0f};

        auto entity = m_Scene->CreateEntity();
        entity.AddComponent<ve::TransformComponent>().SetPosition(position);
        entity.AddComponent<ve::MeshComponent>(m_SphereMesh);
        entity.AddComponent<ve::MaterialPBRComponent>(m_BlueNeon);

        m_Renderer->AddLight(position,
                             m_BlueNeon->GetEmissiveColorFactor(),
                             m_BlueNeon->GetEmissiveStrength());
    }

    {
        auto entity = m_Scene->CreateEntity("Sponza");
        entity.AddComponent<ve::TransformComponent>().SetScale(glm::vec3(0.008f));
        entity.AddComponent<ve::MeshComponent>(m_GLTFSceneMesh);
        entity.AddComponent<ve::MaterialPBRComponent>(m_DefaultMaterial);
    }
}
