#pragma once

#include "VulkanEngine/Assets/Mesh.hpp"
#include "VulkanEngine/Assets/MaterialPBR.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include <string>
#include <cstdint>
#include <vector>
#include <memory>

namespace ve
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

    struct GLTFScene
    {
        std::string name;
        std::string sourceFile;

        std::vector<SceneNode> nodes;
        std::vector<SceneMeshEntry> meshEntries;
        std::vector<std::shared_ptr<MaterialPBR>> materials;
        std::vector<std::shared_ptr<Texture>> textures;

        std::vector<int32_t> rootNodes;
    };

} // namespace ve
