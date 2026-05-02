#pragma once

#include "VulkanEngine/Assets/Mesh.hpp"
#include "VulkanEngine/Assets/MaterialPBR.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include <string>
#include <cstdint>
#include <vector>
#include <memory>

namespace ve::gltf
{
    struct SceneNode
    {
        std::string name;

        glm::vec3 translation{0.0f};
        glm::quat rotation{1.0f, 0.0f, 0.0f, 0.0f};
        glm::vec3 scale{1.0f};

        glm::mat4 localTransform{1.0f};

        int32_t meshIndex = -1;

        int32_t parentIndex = -1;
        std::vector<int32_t> childIndices;
    };

    struct SceneMeshEntry
    {
        std::shared_ptr<Mesh> mesh;
        std::vector<int32_t> materialIndices;
    };

    enum class LightType
    {
        Directional = 0,
        Point = 1,
        Spot = 2,
    };

    struct Light
    {
        std::string name;
        glm::vec3 color{1.0f, 1.0f, 1.0f};
        float intensity{1.0f};
        LightType type;
        float range{0.0f};

        // For spot light
        float innerConeAngle{0.0f};
        float outerConeAngle = glm::radians(45.0f);

        int32_t nodeIndex = -1;
    };

    struct Scene
    {
        std::string name;
        std::string sourceFile;

        std::vector<SceneNode> nodes;
        std::vector<SceneMeshEntry> meshEntries;
        std::vector<std::shared_ptr<MaterialPBR>> materials;
        std::vector<std::shared_ptr<Texture>> textures;

        std::vector<Light> lights;

        std::vector<int32_t> rootNodes;
    };

} // namespace ve
