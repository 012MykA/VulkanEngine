#include "Mesh.hpp"
#include "Backends/Vulkan/VulkanImmediateSubmit.hpp"
#include "Backends/Vulkan/VulkanLogicalDevice.hpp"
#include "VulkanEngine/Core/Log.hpp"
#include "VulkanEngine/Core/Timer.hpp"

#include <tiny_gltf.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <cassert>

namespace ve
{
    void Mesh::SetVertices(std::vector<Vertex> &&vertices)
    {
        m_Vertices = std::move(vertices);
    }

    void Mesh::SetIndices(std::vector<uint32_t> &&indices)
    {
        m_Indices = std::move(indices);
    }

    void Mesh::AddSubMesh(SubMesh subMesh)
    {
        m_SubMeshes.push_back(subMesh);
    }

    void Mesh::ComputeTangents() // Use MikkTSpace algorithm in future
    {
        std::vector<glm::vec3> tangents(m_Vertices.size(), glm::vec3(0.0f));
        std::vector<glm::vec3> bitangents(m_Vertices.size(), glm::vec3(0.0f));

        for (size_t i = 0; i + 2 < m_Indices.size(); i += 3)
        {
            uint32_t i0 = m_Indices[i];
            uint32_t i1 = m_Indices[i + 1];
            uint32_t i2 = m_Indices[i + 2];

            const glm::vec3 &p0 = m_Vertices[i0].position;
            const glm::vec3 &p1 = m_Vertices[i1].position;
            const glm::vec3 &p2 = m_Vertices[i2].position;

            const glm::vec2 &uv0 = m_Vertices[i0].uv;
            const glm::vec2 &uv1 = m_Vertices[i1].uv;
            const glm::vec2 &uv2 = m_Vertices[i2].uv;

            glm::vec3 e1 = p1 - p0;
            glm::vec3 e2 = p2 - p0;

            glm::vec2 d1 = uv1 - uv0;
            glm::vec2 d2 = uv2 - uv0;

            float denom = d1.x * d2.y - d2.x * d1.y;
            if (std::abs(denom) < 1e-6f)
                continue;

            float r = 1.0f / denom;

            glm::vec3 t = (e1 * d2.y - e2 * d1.y) * r;
            glm::vec3 b = (e2 * d1.x - e1 * d2.x) * r;

            tangents[i0] += t;
            tangents[i1] += t;
            tangents[i2] += t;
            bitangents[i0] += b;
            bitangents[i1] += b;
            bitangents[i2] += b;
        }

        for (size_t i = 0; i < m_Vertices.size(); i++)
        {
            const glm::vec3 &n = m_Vertices[i].normal;
            const glm::vec3 &t = tangents[i];

            glm::vec3 tangent = glm::normalize(t - n * glm::dot(n, t));

            float handedness = (glm::dot(glm::cross(n, t), bitangents[i]) < 0.0f)
                                   ? -1.0f
                                   : 1.0f;

            m_Vertices[i].tangent = glm::vec4(tangent, handedness);
        }
    }

    void Mesh::ComputeBounds()
    {
        m_Bounds = AABB{};
        for (const auto &v : m_Vertices)
            m_Bounds.Expand(v.position);

        // SubMesh bounds
        for (auto &sub : m_SubMeshes)
        {
            sub.bounds = AABB{};
            for (uint32_t ii = sub.indexOffset; ii < sub.indexOffset + sub.indexCount; ii++)
            {
                uint32_t vi = m_Indices[ii] + sub.vertexOffset;
                sub.bounds.Expand(m_Vertices[vi].position);
            }
        }
    }

    std::shared_ptr<Mesh> Mesh::Load(const std::string &path)
    {
        Timer timer;

        tinygltf::Model model;
        tinygltf::TinyGLTF loader;
        std::string err;
        std::string warn;

        bool ret = false;
        if (path.ends_with(".glb"))
        {
            ret = loader.LoadBinaryFromFile(&model, &err, &warn, path);
        }
        else
        {
            ret = loader.LoadASCIIFromFile(&model, &err, &warn, path);
        }

        if (!warn.empty())
            VE_CORE_WARN("GLTF Warning: {}", warn);
        if (!err.empty())
            VE_CORE_ERROR("GLTF Error: {}", err);
        if (!ret)
            return nullptr;

        auto mesh = std::make_shared<Mesh>();

        std::string filename = std::filesystem::path(path).filename().string();
        mesh->SetName(filename);

        std::vector<Vertex> allVertices;
        std::vector<uint32_t> allIndices;

        for (const auto &gltfMesh : model.meshes)
        {
            for (const auto &primitive : gltfMesh.primitives)
            {
                SubMesh sub;
                sub.vertexOffset = static_cast<uint32_t>(allVertices.size());
                sub.indexOffset = static_cast<uint32_t>(allIndices.size());
                sub.materialIndex = primitive.material;

                const tinygltf::Accessor &indexAccessor = model.accessors[primitive.indices];
                const tinygltf::BufferView &indexBufferView = model.bufferViews[indexAccessor.bufferView];
                const tinygltf::Buffer &indexBuffer = model.buffers[indexBufferView.buffer];

                sub.indexCount = static_cast<uint32_t>(indexAccessor.count);
                const unsigned char *indexData = &indexBuffer.data[indexBufferView.byteOffset + indexAccessor.byteOffset];

                for (size_t i = 0; i < indexAccessor.count; ++i)
                {
                    if (indexAccessor.componentType == TINYGLTF_PARAMETER_TYPE_UNSIGNED_INT)
                    {
                        allIndices.push_back(((uint32_t *)indexData)[i]);
                    }
                    else if (indexAccessor.componentType == TINYGLTF_PARAMETER_TYPE_UNSIGNED_SHORT)
                    {
                        allIndices.push_back(((uint16_t *)indexData)[i]);
                    }
                }

                const float *posPtr = nullptr;
                const float *normPtr = nullptr;
                const float *uvPtr = nullptr;
                const float *tangentPtr = nullptr;
                size_t vertexCount = 0;

                if (primitive.attributes.count("POSITION"))
                {
                    const tinygltf::Accessor &acc = model.accessors[primitive.attributes.at("POSITION")];
                    const tinygltf::BufferView &bv = model.bufferViews[acc.bufferView];
                    posPtr = reinterpret_cast<const float *>(&(model.buffers[bv.buffer].data[acc.byteOffset + bv.byteOffset]));
                    vertexCount = acc.count;
                }

                if (primitive.attributes.count("NORMAL"))
                {
                    const tinygltf::Accessor &acc = model.accessors[primitive.attributes.at("NORMAL")];
                    const tinygltf::BufferView &bv = model.bufferViews[acc.bufferView];
                    normPtr = reinterpret_cast<const float *>(&(model.buffers[bv.buffer].data[acc.byteOffset + bv.byteOffset]));
                }

                if (primitive.attributes.count("TEXCOORD_0"))
                {
                    const tinygltf::Accessor &acc = model.accessors[primitive.attributes.at("TEXCOORD_0")];
                    const tinygltf::BufferView &bv = model.bufferViews[acc.bufferView];
                    uvPtr = reinterpret_cast<const float *>(&(model.buffers[bv.buffer].data[acc.byteOffset + bv.byteOffset]));
                }

                if (primitive.attributes.count("TANGENT"))
                {
                    const tinygltf::Accessor &acc = model.accessors[primitive.attributes.at("TANGENT")];
                    const tinygltf::BufferView &bv = model.bufferViews[acc.bufferView];
                    tangentPtr = reinterpret_cast<const float *>(&(model.buffers[bv.buffer].data[acc.byteOffset + bv.byteOffset]));
                }

                for (size_t v = 0; v < vertexCount; ++v)
                {
                    Vertex vert{};
                    vert.position = glm::make_vec3(&posPtr[v * 3]);
                    vert.normal = normPtr ? glm::make_vec3(&normPtr[v * 3]) : glm::vec3(0.0f);
                    vert.uv = uvPtr ? glm::make_vec2(&uvPtr[v * 2]) : glm::vec2(0.0f);
                    vert.tangent = tangentPtr ? glm::make_vec4(&tangentPtr[v * 4]) : glm::vec4(0.0f);
                    allVertices.push_back(vert);
                }

                mesh->AddSubMesh(sub);
            }
        }

        mesh->SetVertices(std::move(allVertices));
        mesh->SetIndices(std::move(allIndices));
        mesh->ComputeBounds();

        VE_CORE_INFO("Mesh '{}, loaded ({} ms)", mesh->m_Name, timer.ElapsedMilliseconds());
        VE_CORE_INFO("  Vertices: {}", mesh->m_Vertices.size());
        VE_CORE_INFO("  Indices: {}", mesh->m_Indices.size());

        return mesh;
    }

    void Mesh::UploadToGPU(const VulkanAllocator &allocator, const VulkanImmediateSubmit &upload)
    {
        assert(!m_Vertices.empty() && !m_Indices.empty() && "Mesh::UploadToGPU - mesh must have both vertices and indices");

        const VkDeviceSize vertexSize = sizeof(Vertex) * m_Vertices.size();
        const VkDeviceSize indexSize = sizeof(uint32_t) * m_Indices.size();

        m_IndexCount = static_cast<uint32_t>(m_Indices.size());

        // Staging buffers
        VulkanBuffer stagingVB(allocator, MakeStagingBufferDesc(vertexSize));
        VulkanBuffer stagingIB(allocator, MakeStagingBufferDesc(indexSize));

        stagingVB.Upload(m_Vertices.data(), vertexSize);
        stagingIB.Upload(m_Indices.data(), indexSize);

        // GPU buffers
        m_VertexBuffer = std::make_unique<VulkanBuffer>(allocator, MakeGPUBufferDesc(vertexSize, BufferType::Vertex));

        m_IndexBuffer = std::make_unique<VulkanBuffer>(allocator, MakeGPUBufferDesc(indexSize, BufferType::Index));

        // Coping
        // clang-format off
        upload.Submit([&](VkCommandBuffer cmd)
        {
            stagingVB.CopyTo(cmd, *m_VertexBuffer);
            stagingIB.CopyTo(cmd, *m_IndexBuffer);
        });
        // clang-format on
    }

    void Mesh::FreeCPUData()
    {
        m_Vertices.clear();
        m_Vertices.shrink_to_fit();
        m_Indices.clear();
        m_Indices.shrink_to_fit();
    }

    void Mesh::Bind(VkCommandBuffer cmd) const
    {
        assert((m_VertexBuffer && m_IndexBuffer) && "Mesh::Bind - buffers not uploaded");

        VkBuffer vb = m_VertexBuffer->GetVkHandle();
        VkDeviceSize offset = 0;

        vkCmdBindVertexBuffers(cmd, 0, 1, &vb, &offset);
        vkCmdBindIndexBuffer(cmd, m_IndexBuffer->GetVkHandle(), 0, VK_INDEX_TYPE_UINT32);
    }

} // namespace ve
