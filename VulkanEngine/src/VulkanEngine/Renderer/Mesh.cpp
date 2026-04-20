#include "Mesh.hpp"
#include "Backends/Vulkan/VulkanAllocator.hpp"
#include "Backends/Vulkan/VulkanImmediateSubmit.hpp"

#include <cassert>

namespace ve
{
    Mesh::Mesh(std::vector<Vertex> vertices, std::vector<uint32_t> indices)
        : m_Vertices(std::move(vertices)), m_Indices(std::move(indices))
    {
    }

    void Mesh::Upload(const VulkanAllocator &allocator, const VulkanImmediateSubmit &upload)
    {
        assert(!m_Vertices.empty() && !m_Indices.empty());

        const VkDeviceSize vertexSize = sizeof(Vertex) * m_Vertices.size();
        const VkDeviceSize indexSize = sizeof(uint32_t) * m_Indices.size();

        VulkanBuffer stagingVB(allocator, MakeStagingBufferDesc(vertexSize));
        VulkanBuffer stagingIB(allocator, MakeStagingBufferDesc(indexSize));

        stagingVB.Upload(m_Vertices.data(), vertexSize);
        stagingIB.Upload(m_Indices.data(), indexSize);

        m_VertexBuffer = std::make_unique<VulkanBuffer>(allocator, MakeGPUBufferDesc(vertexSize, BufferType::Vertex));
        m_IndexBuffer = std::make_unique<VulkanBuffer>(allocator, MakeGPUBufferDesc(vertexSize, BufferType::Index));

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

    void Mesh::RecalculateBounds()
    {
        m_MeshBounds = AABB{};

        for (auto& prim : m_Primitives)
        {
            prim.boundingBox = AABB{};

            for (uint32_t i = prim.firstIndex; i < prim.firstIndex + prim.indexCount; i++)
            {
                prim.boundingBox.Expand(m_Vertices[m_Indices[i]].position);
            }
            m_MeshBounds.Expand(prim.boundingBox);
        }
    }

    void Mesh::Bind(VkCommandBuffer cmd) const
    {
        assert(m_VertexBuffer && m_IndexBuffer);

        VkBuffer vb = m_VertexBuffer->GetVkHandle();
        VkDeviceSize offset = 0;

        vkCmdBindVertexBuffers(cmd, 0, 1, &vb, &offset);
        vkCmdBindIndexBuffer(cmd, m_IndexBuffer->GetVkHandle(), 0, VK_INDEX_TYPE_UINT32);
    }

} // namespace ve
