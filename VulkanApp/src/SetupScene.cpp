#include "ExampleLayer.hpp"

#define MAT4_FROM_POS(pos) (glm::translate(glm::mat4(1.0f), pos))

void ExampleLayer::BuildLightTestScene()
{
    ve::GLTFLoader loader(*m_Renderer);
    auto sponza = loader.Load("assets/scenes/KhronosGroup glTF-Sample-Models main 2.0-Sponza/glTF/Sponza.gltf");
    m_SceneRenderer.AddScene(sponza);

    ve::gltf::Light purpleNeon{
        .name = "Puprle Neon",
        .color = {0.9f, 0.0f, 0.8f},
        .intensity = 5.0f,
        .type = ve::gltf::LightType::Point,
    };

    ve::gltf::Light yellowNeon = purpleNeon;
    yellowNeon.name = "Yellow Neon";
    yellowNeon.color = {1.0f, 1.0f, 0.0f};

    ve::gltf::Light blueNeon = purpleNeon;
    blueNeon.name = "Blue Neon";
    blueNeon.color = {0.0f, 0.7f, 1.0f};

    ve::gltf::Light sun{
        .name = "Sun",
        .color = glm::vec3(1.0f),
        .intensity = 1.0f,
        .type = ve::gltf::LightType::Directional,
    };

    m_Renderer->AddLight(purpleNeon, MAT4_FROM_POS(glm::vec3(-3.0f, 2.0f, 0.0f)));
    m_Renderer->AddLight(yellowNeon, MAT4_FROM_POS(glm::vec3(0.0f, 2.0f, 0.0f)));
    m_Renderer->AddLight(blueNeon, MAT4_FROM_POS(glm::vec3(3.0f, 2.0f, 0.0f)));

    glm::vec3 lightPos = {10.0f, 10.0f, 10.0f};
    glm::vec3 center = {0.0f, 0.0f, 0.0f};
    glm::mat4 sunTransform = glm::inverse(glm::lookAt(lightPos, center, glm::vec3(0, 1, 0)));
    m_Renderer->AddLight(sun, sunTransform);
}
