#pragma once

#include "VulkanEngine/Renderer/Mesh.hpp"
#include "VulkanEngine/Renderer/MaterialPBR.hpp"
#include "VulkanEngine/Renderer/Texture.hpp"

#include <tiny_gltf.h>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

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

    struct GLTFLoadOptions
    {
        bool keepCPUData = false;

        TextureFormat baseColorFormat = TextureFormat::RGBA8_SRGB;
        TextureFormat dataFormat = TextureFormat::RGBA8_UNORM;
        TextureFormat emissiveFormat = TextureFormat::RGBA8_SRGB;
    };

    class RendererPBR;

    class GLTFLoader
    {
    public:
        explicit GLTFLoader(RendererPBR &renderer);

        GLTFScene Load(const std::string &path, const GLTFLoadOptions &options = {});

    private:
        void ParseTextures(struct LoadCtx &ctx);
        void ParseMaterials(struct LoadCtx &ctx);
        void ParseMeshes(struct LoadCtx &ctx);
        void ParseNodes(struct LoadCtx &ctx);
        void ComputeLocalTransforms(struct LoadCtx &ctx);
        void BuildSceneRoots(struct LoadCtx &ctx);

        void UploadAll(struct LoadCtx &ctx);

        static std::vector<Vertex> ExtractVertices(const tinygltf::Model &model,
                                                   const tinygltf::Primitive &primitive);

        static std::vector<uint32_t> ExtractIndices(const tinygltf::Model &model,
                                                    const tinygltf::Primitive &primitive);

        static glm::mat4 NodeLocalMatrix(const tinygltf::Node &node);

        RendererPBR &m_Renderer;
    };

} // namespace ve
