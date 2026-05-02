#include "GLTFLoader.hpp"
#include "VulkanEngine/Renderer/RendererPBR.hpp"
#include "VulkanEngine/Core/Log.hpp"
#include "VulkanEngine/Core/Timer.hpp"

#include <glm/gtc/type_ptr.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/matrix_decompose.hpp>

#include <tiny_gltf.h>

#include <cassert>
#include <stdexcept>
#include <unordered_map>
#include <filesystem>

namespace ve
{
    struct LoadCtx
    {
        const GLTFLoadOptions *opts = nullptr;
        tinygltf::Model gltf;
        gltf::Scene scene;
    };

    GLTFLoader::GLTFLoader(RendererPBR &renderer) : m_Renderer(renderer) {}

    gltf::Scene GLTFLoader::Load(const std::string &path, const GLTFLoadOptions &options)
    {
        Timer loadingTimer;

        LoadCtx ctx;
        ctx.opts = &options;

        tinygltf::TinyGLTF loader;
        std::string err, warn;
        bool ok = path.ends_with(".glb")
                      ? loader.LoadBinaryFromFile(&ctx.gltf, &err, &warn, path)
                      : loader.LoadASCIIFromFile(&ctx.gltf, &err, &warn, path);

        // clang-format off
        if (!warn.empty())  VE_CORE_WARN("GLTF Warning: {}", warn);
        if (!err.empty())   VE_CORE_ERROR("GLTF Error: {}", err);
        if (!ok)            throw std::runtime_error("Failed to load: " + path);
        // clang-format on

        ctx.scene.sourceFile = path;
        ctx.scene.name = ctx.gltf.scenes[ctx.gltf.defaultScene >= 0 ? ctx.gltf.defaultScene : 0].name;

        ParseTextures(ctx);
        ParseMaterials(ctx);
        ParseMeshes(ctx);
        ParseNodes(ctx);
        ComputeLocalTransforms(ctx);
        BuildSceneRoots(ctx);

        if (options.autoUpload)
            UploadAll(ctx);

        VE_CORE_TRACE("GLTFScene '{}' loaded ({} ms)",
                      std::filesystem::path(ctx.scene.sourceFile).filename().string(),
                      loadingTimer.ElapsedMilliseconds());

        return std::move(ctx.scene);
    }

    void GLTFLoader::UploadScene(gltf::Scene &scene, const GLTFLoadOptions &options)
    {
        LoadCtx ctx{
            .opts = &options,
            .scene = std::move(scene),
        };

        UploadAll(ctx);

        scene = std::move(ctx.scene);
    }

    // -------------------------------------------------------
    // ParseLights
    // -------------------------------------------------------
    void GLTFLoader::ParseLights(LoadCtx &ctx)
    {
        const auto &gltf = ctx.gltf;

        if (!gltf.extensions.count("KHR_lights_punctual"))
            return;

        const auto &ext = gltf.extensions.at("KHR_lights_punctual");
        if (!ext.Has("lights"))
            return;

        const auto &lightsArray = ext.Get("lights");

        for (size_t i = 0; i < lightsArray.ArrayLen(); ++i)
        {
            const auto &lightVal = lightsArray.Get(static_cast<int>(i));
            ve::gltf::Light light{};

            if (lightVal.Has("name"))
                light.name = lightVal.Get("name").Get<std::string>();

            if (lightVal.Has("color"))
            {
                auto c = lightVal.Get("color");
                light.color = glm::vec3(c.Get(0).Get<double>(), c.Get(1).Get<double>(), c.Get(2).Get<double>());
            }

            if (lightVal.Has("intensity"))
            {
                light.intensity = static_cast<float>(lightVal.Get("intensity").Get<double>());
            }

            if (lightVal.Has("range"))
            {
                light.range = static_cast<float>(lightVal.Get("range").Get<double>());
            }

            std::string type = lightVal.Get("type").Get<std::string>();
            if (type == "directional")
            {
                light.type = gltf::LightType::Directional;
            }
            else if (type == "point")
            {
                light.type = gltf::LightType::Point;
            }
            else if (type == "spot")
            {
                light.type = gltf::LightType::Spot;

                if (lightVal.Has("spot"))
                {
                    const auto &spot = lightVal.Get("spot");
                    if (spot.Has("innerConeAngle"))
                        light.innerConeAngle = static_cast<float>(spot.Get("innerConeAngle").Get<double>());
                    if (spot.Has("outerConeAngle"))
                        light.outerConeAngle = static_cast<float>(spot.Get("outerConeAngle").Get<double>());
                }
            }

            ctx.scene.lights.push_back(light);
        }
    }

    // -------------------------------------------------------
    // ParseTextures
    // -------------------------------------------------------
    void GLTFLoader::ParseTextures(LoadCtx &ctx)
    {
        const auto &gltf = ctx.gltf;
        ctx.scene.textures.reserve(gltf.textures.size());

        for (const auto &gltfTex : gltf.textures)
        {
            if (gltfTex.source < 0)
            {
                ctx.scene.textures.push_back(nullptr);
                continue;
            }

            const auto &image = gltf.images[gltfTex.source];

            if (image.image.empty())
            {
                ctx.scene.textures.push_back(nullptr);
                continue;
            }

            TextureDesc desc{
                .pixels = image.image.data(),
                .width = static_cast<uint32_t>(image.width),
                .height = static_cast<uint32_t>(image.height),
                .format = TextureFormat::RGBA8_SRGB, // overridden in ParseMaterials via RebakeTexture
                .generateMips = true,
            };

            ctx.scene.textures.push_back(std::make_shared<Texture>(desc));
        }
    }

    // -------------------------------------------------------
    // RebakeTexture helper
    // -------------------------------------------------------
    namespace
    {
        std::shared_ptr<Texture> RebakeTexture(const std::vector<std::shared_ptr<Texture>> &textures,
                                               const std::vector<tinygltf::Texture> &gltfTextures,
                                               const tinygltf::Model &gltf,
                                               int texIndex,
                                               TextureFormat desiredFormat)
        {
            if (texIndex < 0 || texIndex >= static_cast<int>(textures.size()))
                return nullptr;

            auto tex = textures[texIndex];
            if (!tex)
                return nullptr;

            const int source = gltfTextures[texIndex].source;
            if (source < 0)
                return tex;

            const auto &image = gltf.images[source];
            if (image.image.empty())
                return tex;

            TextureDesc desc{
                .pixels = image.image.data(),
                .width = static_cast<uint32_t>(image.width),
                .height = static_cast<uint32_t>(image.height),
                .format = desiredFormat,
                .generateMips = true,
            };

            return std::make_shared<Texture>(desc);
        }
    }

    // -------------------------------------------------------
    // ParseMaterials
    // -------------------------------------------------------
    void GLTFLoader::ParseMaterials(LoadCtx &ctx)
    {
        const auto &gltf = ctx.gltf;
        const auto &opts = *ctx.opts;
        ctx.scene.materials.reserve(gltf.materials.size());

        for (const auto &gltfMat : gltf.materials)
        {
            auto mat = std::make_shared<MaterialPBR>();
            mat->SetName(gltfMat.name);

            const auto &pbr = gltfMat.pbrMetallicRoughness;

            if (pbr.baseColorFactor.size() == 4)
            {
                mat->SetBaseColorFactor(glm::vec4(
                    pbr.baseColorFactor[0],
                    pbr.baseColorFactor[1],
                    pbr.baseColorFactor[2],
                    pbr.baseColorFactor[3]));
            }

            if (pbr.baseColorTexture.index >= 0)
            {
                auto tex = RebakeTexture(ctx.scene.textures, gltf.textures, gltf,
                                         pbr.baseColorTexture.index, opts.baseColorFormat);
                if (tex)
                    mat->SetBaseColorMap(std::move(tex));
            }

            mat->SetMetallicFactor(static_cast<float>(pbr.metallicFactor));
            mat->SetRoughnessFactor(static_cast<float>(pbr.roughnessFactor));

            if (pbr.metallicRoughnessTexture.index >= 0)
            {
                auto tex = RebakeTexture(ctx.scene.textures, gltf.textures, gltf,
                                         pbr.metallicRoughnessTexture.index, opts.dataFormat);
                if (tex)
                    mat->SetAoMetallicRoughnessMap(std::move(tex));
            }

            if (gltfMat.normalTexture.index >= 0)
            {
                auto tex = RebakeTexture(ctx.scene.textures, gltf.textures, gltf,
                                         gltfMat.normalTexture.index, opts.dataFormat);
                if (tex)
                {
                    mat->SetNormalMap(std::move(tex));
                    mat->SetNormalScale(static_cast<float>(gltfMat.normalTexture.scale));
                }
            }

            if (gltfMat.occlusionTexture.index >= 0)
                mat->SetOcclusionStrength(static_cast<float>(gltfMat.occlusionTexture.strength));

            if (gltfMat.emissiveFactor.size() == 3)
            {
                mat->SetEmissiveColorFactor(glm::vec3(
                    gltfMat.emissiveFactor[0],
                    gltfMat.emissiveFactor[1],
                    gltfMat.emissiveFactor[2]));
            }

            {
                auto it = gltfMat.extensions.find("KHR_materials_emissive_strength");
                if (it != gltfMat.extensions.end() && it->second.Has("emissiveStrength"))
                    mat->SetEmissiveStrength(static_cast<float>(it->second.Get("emissiveStrength").GetNumberAsDouble()));
            }

            if (gltfMat.emissiveTexture.index >= 0)
            {
                auto tex = RebakeTexture(ctx.scene.textures, gltf.textures, gltf,
                                         gltfMat.emissiveTexture.index, opts.emissiveFormat);
                if (tex)
                    mat->SetEmissiveMap(std::move(tex));
            }

            if (gltfMat.alphaMode == "OPAQUE")
                mat->SetAlphaMode(AlphaMode::Opaque);
            else if (gltfMat.alphaMode == "MASK")
            {
                mat->SetAlphaMode(AlphaMode::Mask);
                mat->SetAlphaCutoff(static_cast<float>(gltfMat.alphaCutoff));
            }
            else if (gltfMat.alphaMode == "BLEND")
                mat->SetAlphaMode(AlphaMode::Blend);

            mat->SetDoubleSided(gltfMat.doubleSided);

            ctx.scene.materials.push_back(std::move(mat));
        }
    }

    // -------------------------------------------------------
    // Accessor helpers
    // -------------------------------------------------------
    namespace
    {
        const float *AccessorDataF(const tinygltf::Model &model,
                                   const tinygltf::Primitive &prim,
                                   const char *attribute)
        {
            auto it = prim.attributes.find(attribute);
            if (it == prim.attributes.end())
                return nullptr;
            const auto &acc = model.accessors[it->second];
            const auto &view = model.bufferViews[acc.bufferView];
            const auto &buf = model.buffers[view.buffer];
            return reinterpret_cast<const float *>(buf.data.data() + view.byteOffset + acc.byteOffset);
        }

        size_t AccessorCount(const tinygltf::Model &model,
                             const tinygltf::Primitive &prim,
                             const char *attribute)
        {
            auto it = prim.attributes.find(attribute);
            if (it == prim.attributes.end())
                return 0;
            return model.accessors[it->second].count;
        }
    }

    // -------------------------------------------------------
    // ParseMeshes
    // -------------------------------------------------------
    void GLTFLoader::ParseMeshes(LoadCtx &ctx)
    {
        const auto &gltf = ctx.gltf;
        ctx.scene.meshEntries.reserve(gltf.meshes.size());

        for (const auto &gltfMesh : gltf.meshes)
        {
            std::vector<Vertex> allVertices;
            std::vector<uint32_t> allIndices;
            std::vector<int32_t> matIndices;

            auto mesh = std::make_shared<Mesh>();
            mesh->SetName(gltfMesh.name);

            for (const auto &gltfPrim : gltfMesh.primitives)
            {
                if (gltfPrim.mode != TINYGLTF_MODE_TRIANGLES)
                    continue;

                auto vertices = ExtractVertices(gltf, gltfPrim);
                auto indices = ExtractIndices(gltf, gltfPrim);

                if (vertices.empty() || indices.empty())
                    continue;

                Primitive prim{
                    .firstIndex = static_cast<uint32_t>(allIndices.size()),
                    .indexCount = static_cast<uint32_t>(indices.size()),
                    .materialIndex = gltfPrim.material,
                };

                uint32_t vertexOffset = static_cast<uint32_t>(allVertices.size());
                for (auto &idx : indices)
                    idx += vertexOffset;

                allVertices.insert(allVertices.end(), vertices.begin(), vertices.end());
                allIndices.insert(allIndices.end(), indices.begin(), indices.end());

                mesh->AddPrimitive(prim);
                matIndices.push_back(gltfPrim.material);
            }

            mesh->SetVertices(std::move(allVertices));
            mesh->SetIndices(std::move(allIndices));
            mesh->RecalculateBounds();

            ctx.scene.meshEntries.push_back(gltf::SceneMeshEntry{
                .mesh = std::move(mesh),
                .materialIndices = std::move(matIndices),
            });
        }
    }

    // -------------------------------------------------------
    // ExtractVertices
    // -------------------------------------------------------
    std::vector<Vertex> GLTFLoader::ExtractVertices(const tinygltf::Model &model,
                                                    const tinygltf::Primitive &primitive)
    {
        const float *positions = AccessorDataF(model, primitive, "POSITION");
        const float *normals = AccessorDataF(model, primitive, "NORMAL");
        const float *tangents = AccessorDataF(model, primitive, "TANGENT");
        const float *uvs = AccessorDataF(model, primitive, "TEXCOORD_0");

        size_t count = AccessorCount(model, primitive, "POSITION");
        if (!positions || count == 0)
            return {};

        std::vector<Vertex> vertices(count);
        for (size_t i = 0; i < count; i++)
        {
            vertices[i].position = {positions[i * 3], positions[i * 3 + 1], positions[i * 3 + 2]};

            vertices[i].normal = normals
                                     ? glm::vec3(normals[i * 3], normals[i * 3 + 1], normals[i * 3 + 2])
                                     : glm::vec3(0.0f, 1.0f, 0.0f);

            vertices[i].tangent = tangents
                                      ? glm::vec4(tangents[i * 4], tangents[i * 4 + 1], tangents[i * 4 + 2], tangents[i * 4 + 3])
                                      : glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);

            vertices[i].uv = uvs
                                 ? glm::vec2(uvs[i * 2], uvs[i * 2 + 1])
                                 : glm::vec2(0.0f);
        }
        return vertices;
    }

    // -------------------------------------------------------
    // ExtractIndices
    // -------------------------------------------------------
    std::vector<uint32_t> GLTFLoader::ExtractIndices(const tinygltf::Model &model,
                                                     const tinygltf::Primitive &primitive)
    {
        if (primitive.indices < 0)
            return {};

        const auto &acc = model.accessors[primitive.indices];
        const auto &view = model.bufferViews[acc.bufferView];
        const auto &buf = model.buffers[view.buffer];
        const uint8_t *raw = buf.data.data() + view.byteOffset + acc.byteOffset;

        std::vector<uint32_t> indices(acc.count);

        switch (acc.componentType)
        {
        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
            for (size_t i = 0; i < acc.count; i++)
                indices[i] = reinterpret_cast<const uint32_t *>(raw)[i];
            break;
        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
            for (size_t i = 0; i < acc.count; i++)
                indices[i] = reinterpret_cast<const uint16_t *>(raw)[i];
            break;
        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
            for (size_t i = 0; i < acc.count; i++)
                indices[i] = raw[i];
            break;
        default:
            break;
        }
        return indices;
    }

    // -------------------------------------------------------
    // ParseNodes
    // -------------------------------------------------------
    void GLTFLoader::ParseNodes(LoadCtx &ctx)
    {
        const auto &gltf = ctx.gltf;
        ctx.scene.nodes.resize(gltf.nodes.size());

        for (size_t i = 0; i < gltf.nodes.size(); i++)
        {
            const auto &gltfNode = gltf.nodes[i];
            auto &node = ctx.scene.nodes[i];

            node.name = gltfNode.name;
            node.meshIndex = gltfNode.mesh;
            node.parentIndex = -1;

            if (!gltfNode.translation.empty())
                node.translation = glm::vec3(gltfNode.translation[0], gltfNode.translation[1], gltfNode.translation[2]);

            if (!gltfNode.rotation.empty())
                node.rotation = glm::quat(
                    static_cast<float>(gltfNode.rotation[3]),
                    static_cast<float>(gltfNode.rotation[0]),
                    static_cast<float>(gltfNode.rotation[1]),
                    static_cast<float>(gltfNode.rotation[2]));

            if (!gltfNode.scale.empty())
                node.scale = glm::vec3(gltfNode.scale[0], gltfNode.scale[1], gltfNode.scale[2]);

            node.localTransform = NodeLocalMatrix(gltfNode);

            node.childIndices.reserve(gltfNode.children.size());
            for (int child : gltfNode.children)
            {
                node.childIndices.push_back(child);
                ctx.scene.nodes[child].parentIndex = static_cast<int32_t>(i);
            }
        }
    }

    void GLTFLoader::ComputeLocalTransforms(LoadCtx &)
    {
        // localTransform already built in ParseNodes via NodeLocalMatrix()
    }

    void GLTFLoader::BuildSceneRoots(LoadCtx &ctx)
    {
        const auto &gltf = ctx.gltf;

        if (!gltf.scenes.empty() && ctx.gltf.defaultScene >= 0)
        {
            const auto &gltfScene = gltf.scenes[ctx.gltf.defaultScene];
            ctx.scene.rootNodes.reserve(gltfScene.nodes.size());
            for (int n : gltfScene.nodes)
                ctx.scene.rootNodes.push_back(n);
        }
        else
        {
            for (size_t i = 0; i < ctx.scene.nodes.size(); i++)
                if (ctx.scene.nodes[i].parentIndex == -1)
                    ctx.scene.rootNodes.push_back(static_cast<int32_t>(i));
        }
    }

    // -------------------------------------------------------
    // NodeLocalMatrix
    // -------------------------------------------------------
    glm::mat4 GLTFLoader::NodeLocalMatrix(const tinygltf::Node &node)
    {
        if (!node.matrix.empty())
            return glm::make_mat4(node.matrix.data()); // было: mode.matrix.data()

        glm::mat4 T(1.0f), R(1.0f), S(1.0f);

        if (!node.translation.empty())
            T = glm::translate(glm::mat4(1.0f), glm::vec3(node.translation[0], node.translation[1], node.translation[2]));

        if (!node.rotation.empty())
        {
            glm::quat q(static_cast<float>(node.rotation[3]),
                        static_cast<float>(node.rotation[0]),
                        static_cast<float>(node.rotation[1]),
                        static_cast<float>(node.rotation[2]));
            R = glm::mat4_cast(q);
        }

        if (!node.scale.empty())
            S = glm::scale(glm::mat4(1.0f), glm::vec3(node.scale[0], node.scale[1], node.scale[2]));

        return T * R * S;
    }

    // -------------------------------------------------------
    // UploadAll
    // -------------------------------------------------------
    void GLTFLoader::UploadAll(LoadCtx &ctx)
    {
        const bool keepCPU = ctx.opts->keepCPUData;

        std::unordered_map<Texture *, bool> uploadedTextures;

        auto uploadTexIfNeeded = [&](const std::shared_ptr<Texture> &tex)
        {
            if (!tex || tex->IsUploaded())
                return;
            auto [it, inserted] = uploadedTextures.emplace(tex.get(), true);
            if (inserted)
                m_Renderer.UploadTexture(*tex, !keepCPU);
        };

        for (auto &mat : ctx.scene.materials)
        {
            if (!mat)
                continue;
            uploadTexIfNeeded(mat->GetBaseColorMap());
            uploadTexIfNeeded(mat->GetNormalMap());
            uploadTexIfNeeded(mat->GetAoMetallicRoughnessMap());
            uploadTexIfNeeded(mat->GetEmissiveMap());
        }

        for (auto &mat : ctx.scene.materials)
            if (mat)
                m_Renderer.UploadMaterial(*mat);

        for (auto &entry : ctx.scene.meshEntries)
            if (entry.mesh)
                m_Renderer.UploadMesh(*entry.mesh, !keepCPU);
    }

} // namespace ve
