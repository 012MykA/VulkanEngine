#pragma once

#include "VulkanEngine/Assets/GLTFScene.hpp"

#include <tiny_gltf.h>

#include <cstdint>
#include <string>
#include <vector>

namespace ve
{
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

        gltf::Scene Load(const std::string &path, const GLTFLoadOptions &options = {});

    private:
        void ParseLights(struct LoadCtx &ctx);
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
