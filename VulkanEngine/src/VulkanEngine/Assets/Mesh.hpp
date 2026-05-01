#pragma once

#include "Backends/Vulkan/VulkanBuffer.hpp"

#include "VulkanEngine/Renderer/AABB.hpp"
#include "VulkanEngine/Assets/Vertex.hpp"

#include <vulkan/vulkan.h>

#include <cstdint>
#include <vector>
#include <memory>
#include <string>

namespace ve
{
    struct Primitive
    {
        uint32_t firstIndex;
        uint32_t indexCount;
        int32_t materialIndex = -1;
        AABB boundingBox;
    };

    class VulkanAllocator;
    class VulkanImmediateSubmit;

    class Mesh
    {
    public:
        Mesh() = default;
        Mesh(std::vector<Vertex> vertices, std::vector<uint32_t> indices);

        ~Mesh() = default;

        Mesh(const Mesh &) = delete;
        Mesh &operator=(const Mesh &) = delete;

        Mesh(Mesh &&) noexcept = default;
        Mesh &operator=(Mesh &&) noexcept = default;

    public:
        // Load data onto the GPU. After this, the data on the CPU can be released.
        void Upload(const VulkanAllocator &allocator, const VulkanImmediateSubmit &upload);

        // Clears vertices and indices stored in object
        void FreeCPUData();

        // Binds vertex and index buffers
        void Bind(VkCommandBuffer cmd) const;

        // --- Setters ---

        void SetVertices(std::vector<Vertex> vertices) { m_Vertices = std::move(vertices); }
        void SetIndices(std::vector<uint32_t> indices) { m_Indices = std::move(indices); }
        void AddPrimitive(const Primitive &primitive) { m_Primitives.push_back(primitive); }
        void SetName(const std::string &name) { m_Name = name; }
        void RecalculateBounds();

        // --- Getters ---

        const std::vector<Vertex> &GetVertices() const { return m_Vertices; }
        const std::vector<uint32_t> &GetIndices() const { return m_Indices; }
        const std::vector<Primitive> &GetPrimitives() const { return m_Primitives; }
        const std::string &GetName() const { return m_Name; }
        const AABB &GetBoundingBox() const { return m_MeshBounds; }
        bool IsUploaded() const { return m_VertexBuffer != nullptr; }

    private:
        // --- CPU data ---
        std::vector<Vertex> m_Vertices;
        std::vector<uint32_t> m_Indices;

        std::vector<Primitive> m_Primitives;

        std::string m_Name;
        AABB m_MeshBounds;

        std::unique_ptr<VulkanBuffer> m_VertexBuffer;
        std::unique_ptr<VulkanBuffer> m_IndexBuffer;
    };

} // namespace ve
