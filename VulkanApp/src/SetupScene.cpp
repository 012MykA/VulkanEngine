#include "ExampleLayer.hpp"

#include <glm/gtc/matrix_transform.hpp>

void ExampleLayer::SetupScene()
{
    m_Scene = std::make_unique<ve::Scene>();
    
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

    m_YellowNeon = std::make_shared<ve::MaterialPBR>();
    m_YellowNeon->SetName("Yellow neon");
    m_YellowNeon->SetEmissiveColorFactor({1.0f, 1.0f, 0.0f});
    m_YellowNeon->SetEmissiveStrength(5.0f);
    m_Renderer->UploadMaterial(*m_YellowNeon);

    m_BlueNeon = std::make_shared<ve::MaterialPBR>();
    m_BlueNeon->SetName("Blue neon");
    m_BlueNeon->SetEmissiveColorFactor({0.0f, 0.7f, 1.0f});
    m_BlueNeon->SetEmissiveStrength(5.0f);
    m_Renderer->UploadMaterial(*m_BlueNeon);
}

void ExampleLayer::SetupObjects()
{
    {
        glm::vec3 position = {-3.0f, 2.0f, 0.0f};

        auto entity = m_Scene->CreateEntity();
        entity.AddComponent<ve::TransformComponent>().SetPosition(position);
        entity.AddComponent<ve::MeshComponent>(m_SphereMesh);
        entity.AddComponent<ve::MaterialPBRComponent>(m_PurpleNeon);
    }

    {
        glm::vec3 position = {0.0f, 2.0f, 0.0f};
        
        auto entity = m_Scene->CreateEntity();
        entity.AddComponent<ve::TransformComponent>().SetPosition(position);
        entity.AddComponent<ve::MeshComponent>(m_SphereMesh);
        entity.AddComponent<ve::MaterialPBRComponent>(m_YellowNeon);
    }

    {
        glm::vec3 position = {3.0f, 2.0f, 0.0f};

        auto entity = m_Scene->CreateEntity();
        entity.AddComponent<ve::TransformComponent>().SetPosition(position);
        entity.AddComponent<ve::MeshComponent>(m_SphereMesh);
        entity.AddComponent<ve::MaterialPBRComponent>(m_BlueNeon);
    }
}
