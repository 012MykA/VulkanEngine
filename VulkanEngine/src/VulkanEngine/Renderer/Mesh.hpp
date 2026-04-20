#pragma once

#include "Backends/Vulkan/VulkanBuffer.hpp"
#include "Backends/Vulkan/VulkanAllocator.hpp"
#include "VulkanEngine/Renderer/Primitives/Vertex.hpp"
#include "VulkanEngine/Renderer/Primitives/AABB.hpp"

#include <cstdint>
#include <string>
#include <memory>
#include <vector>

namespace ve
{
    class VulkanImmediateSubmit;
    class VulkanLogicalDevice;

    struct SubMesh
    {
        uint32_t indexOffset = 0;
        uint32_t indexCount = 0;
        uint32_t vertexOffset = 0;
        int32_t materialIndex = -1;
        AABB bounds;
    };

    class Mesh
    {
    public:
        Mesh() = default;
        ~Mesh() = default;

        Mesh(const Mesh &) = delete;
        Mesh &operator=(const Mesh &) = delete;

        Mesh(Mesh &&) noexcept = default;
        Mesh &operator=(Mesh &&) noexcept = default;

        static std::shared_ptr<Mesh> Load(const std::string &path);

    public:
        void UploadToGPU(const VulkanAllocator &allocator, const VulkanImmediateSubmit &upload);
        void FreeCPUData();

        void Bind(VkCommandBuffer cmd) const;

        void ComputeTangents();
        void ComputeBounds();

    public: // Setters
        void SetVertices(std::vector<Vertex> &&vertices);
        void SetIndices(std::vector<uint32_t> &&indices);
        void AddSubMesh(SubMesh subMesh);

    public:
        // Getters
        const std::vector<SubMesh> &GetSubMeshes() const { return m_SubMeshes; }
        const AABB &GetBounds() const { return m_Bounds; }
        bool IsUploaded() const { return m_VertexBuffer != nullptr; }
        const std::vector<Vertex> &GetVertices() const { return m_Vertices; }
        const std::vector<uint32_t> &GetIndices() const { return m_Indices; }
        const std::string &GetName() const { return m_Name; }
        void SetName(std::string name) { m_Name = std::move(name); }

        uint32_t GetIndexCount() const { return m_IndexCount; }

    private:
        // CPU Data
        std::vector<Vertex> m_Vertices;
        std::vector<uint32_t> m_Indices;
        std::vector<SubMesh> m_SubMeshes;
        AABB m_Bounds;
        std::string m_Name;

        uint32_t m_IndexCount = 0;

        // GPU Data
        std::unique_ptr<VulkanBuffer> m_VertexBuffer;
        std::unique_ptr<VulkanBuffer> m_IndexBuffer;
    };

} // namespace ve
