#include "MeshLoader.hpp"
#include "VulkanEngine/Core/Log.hpp"
#include "VulkanEngine/Core/Timer.hpp"

#include <tiny_gltf.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <filesystem>

namespace ve
{
    std::shared_ptr<Mesh> MeshLoader::LoadGLTF(const std::string &path)
    {
        Timer loadingTimer;

        tinygltf::Model model;
        tinygltf::TinyGLTF loader;
        std::string err;
        std::string warn;

        // Loading
        bool ret = path.ends_with(".glb")
                       ? loader.LoadBinaryFromFile(&model, &err, &warn, path)
                       : loader.LoadASCIIFromFile(&model, &err, &warn, path);

        // clang-format off
        if (!warn.empty())  VE_CORE_WARN("GLTF Warning: {}", warn);
        if (!err.empty())   VE_CORE_ERROR("GLTF Error: {}", err);
        if (!ret)           return nullptr;
        // clang-format on

        auto mesh = std::make_shared<Mesh>();

        std::string filename = std::filesystem::path(path).filename().string();
        mesh->SetName(filename);

        std::vector<Vertex> allVertices;
        std::vector<uint32_t> allIndices;

        for (const auto &gltfMesh : model.meshes)
        {
            for (const auto &gltfPrimitive : gltfMesh.primitives)
            {
                Primitive prim{};
                prim.firstIndex = static_cast<uint32_t>(allIndices.size());
                prim.materialIndex = gltfPrimitive.material;

                auto getAccessorData = [&](const std::string &attrName) -> const float *
                {
                    if (gltfPrimitive.attributes.find(attrName) == gltfPrimitive.attributes.end())
                        return nullptr;

                    const auto &accessor = model.accessors[gltfPrimitive.attributes.at(attrName)];
                    const auto &bufferView = model.bufferViews[accessor.bufferView];
                    const auto &buffer = model.buffers[bufferView.buffer];
                    return reinterpret_cast<const float *>(&(buffer.data[accessor.byteOffset + bufferView.byteOffset]));
                };

                const float *positions = getAccessorData("POSITION");
                const float *normals = getAccessorData("NORMAL");
                const float *tangents = getAccessorData("TANGENT");
                const float *texCoords = getAccessorData("TEXCOORD_0");

                size_t vertexCount = model.accessors[gltfPrimitive.attributes.at("POSITION")].count;
                uint32_t vertexStartOffset = static_cast<uint32_t>(allVertices.size());

                // Vertices
                for (size_t v = 0; v < vertexCount; v++)
                {
                    Vertex vertex{
                        .position = glm::make_vec3(&positions[v * 3]),
                        .normal = normals ? glm::make_vec3(&normals[v * 3]) : glm::vec3(0.0f, 1.0f, 0.0f),
                        .tangent = tangents ? glm::make_vec4(&tangents[v * 4]) : glm::vec4(0.0f),
                        .uv = texCoords ? glm::make_vec2(&texCoords[v * 2]) : glm::vec2(0.0f),
                    };

                    allVertices.push_back(vertex);
                    prim.boundingBox.Expand(vertex.position);
                }

                // Indices
                if (gltfPrimitive.indices >= 0)
                {
                    const auto &accessor = model.accessors[gltfPrimitive.indices];
                    const auto &bufferView = model.bufferViews[accessor.bufferView];
                    const auto &buffer = model.buffers[bufferView.buffer];
                    prim.indexCount = static_cast<uint32_t>(accessor.count);

                    const void *dataPtr = &(buffer.data[accessor.byteOffset + bufferView.byteOffset]);

                    for (size_t i = 0; i < accessor.count; i++)
                    {
                        uint32_t index = 0;
                        if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT)
                            index = static_cast<const uint32_t *>(dataPtr)[i];
                        else if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT)
                            index = static_cast<const uint16_t *>(dataPtr)[i];
                        else if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE)
                            index = static_cast<const uint8_t *>(dataPtr)[i];

                        allIndices.push_back(index + vertexStartOffset);
                    }
                }
                else
                {
                    prim.indexCount = static_cast<uint32_t>(vertexCount);
                    for (uint32_t i = 0; i < prim.indexCount; i++)
                        allIndices.push_back(i + vertexStartOffset);
                }

                mesh->AddPrimitive(prim);
            }
        }

        mesh->SetVertices(std::move(allVertices));
        mesh->SetIndices(std::move(allIndices));
        mesh->RecalculateBounds();

        VE_CORE_TRACE("Mesh '{}' loaded (Vertices: {}, Indices: {}, {} ms)",
                      mesh->GetName(),
                      mesh->GetVertices().size(),
                      mesh->GetIndices().size(),
                      loadingTimer.ElapsedMilliseconds());

        return std::move(mesh);
    }

    std::future<std::shared_ptr<Mesh>> MeshLoader::LoadGLTFAsync(const std::string &path)
    {
        return std::async(std::launch::async, [this, path]()
                          { return LoadGLTF(path); });
    }

} // namespace ve
