#include "ExampleLayer.hpp"

#define MAT4_FROM_POS(pos) (glm::translate(glm::mat4(1), pos))

void ExampleLayer::BuildLightTestScene()
{
    ve::GLTFLoader loader(*m_Renderer);

    for (int i = 0; i < 10; i += 2)
    {
        for (int j = 0; j < 6; j += 2)
        {
            m_LoadingScenes.push_back(PendingScene{
                .scene = loader.LoadAsync("assets/scenes/DamagedHelmet/glTF/DamagedHelmet.gltf"),
                .transform = glm::rotate(
                    glm::translate(glm::mat4(1.0f), glm::vec3(-7 + i, 1, -1 + j)),
                    glm::radians(180.0f),
                    glm::vec3(0, 1, 0)),
            });
        }
    }

    m_LoadingScenes.push_back(PendingScene{
        .scene = loader.LoadAsync("assets/scenes/Sponza/glTF/Sponza.gltf"),
        .transform = glm::mat4(1.0f),
    });

    ve::gltf::Light purpleNeon{
        .name = "Puprle Neon",
        .color = {0.9f, 0, 0.8f},
        .intensity = 3,
        .type = ve::gltf::LightType::Point,
    };
    m_Renderer->AddLight(purpleNeon, MAT4_FROM_POS(glm::vec3(-3, 1, 3.5f)));

    ve::gltf::Light yellowNeon = purpleNeon;
    yellowNeon.name = "Yellow Neon";
    yellowNeon.color = {1, 1, 0};
    m_Renderer->AddLight(yellowNeon, MAT4_FROM_POS(glm::vec3(0, 1, 3.5f)));

    ve::gltf::Light blueNeon = purpleNeon;
    blueNeon.name = "Blue Neon";
    blueNeon.color = {0, 0.7f, 1};
    m_Renderer->AddLight(blueNeon, MAT4_FROM_POS(glm::vec3(3, 1, 3.5f)));

    ve::gltf::Light sun{
        .name = "Sun",
        .color = {1, 0.6f, 0.3f},
        .intensity = 4.5f,
        .type = ve::gltf::LightType::Directional,
    };
    glm::vec3 lightPos = {20.0f, 3.5f, -15.0f};
    glm::vec3 center = {0, 0, 0};
    glm::mat4 sunTransform = glm::inverse(glm::lookAt(lightPos, center, glm::vec3(0, 1, 0)));
    m_Renderer->AddLight(sun, sunTransform);

    ve::gltf::Light spot{
        .name = "Spot Light",
        .color = {0.1f, 0.5f, 0.2f},
        .intensity = 75,
        .type = ve::gltf::LightType::Spot,
    };

    glm::mat4 spotTransform = glm::rotate(
        glm::translate(glm::mat4(1.0f), glm::vec3(-7, 2, -0.2f)),
        glm::radians(90.0f),
        glm::vec3(0, 1, 0));
    m_Renderer->AddLight(spot, spotTransform);
}
