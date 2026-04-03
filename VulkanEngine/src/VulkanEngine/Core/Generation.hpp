#pragma once

#include "VulkanEngine/Renderer/Mesh.hpp"

namespace ve
{
    float GetHeight(float x, float z)
    {
        float scale = 0.05f;

        float h = sinf(x * scale) * cosf(z * scale);
        return h * 5.0f;
    }

    std::shared_ptr<Mesh> GenerateTerrain(int size, float spacing)
    {
        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;

        int vertCount = size + 1;

        vertices.reserve(vertCount * vertCount);

        for (int z = 0; z <= size; z++)
        {
            for (int x = 0; x <= size; x++)
            {
                float worldX = x * spacing;
                float worldZ = z * spacing;

                float y = GetHeight(worldX, worldZ);

                Vertex v{};
                v.position = {worldX, y, worldZ};
                v.uv = {x / (float)size, z / (float)size};

                v.normal = {0.0f, 1.0f, 0.0f};

                vertices.push_back(v);
            }
        }

        for (int z = 0; z < size; z++)
        {
            for (int x = 0; x < size; x++)
            {
                int i0 = z * (size + 1) + x;
                int i1 = i0 + 1;
                int i2 = i0 + (size + 1);
                int i3 = i2 + 1;

                indices.push_back(i0);
                indices.push_back(i2);
                indices.push_back(i1);

                indices.push_back(i1);
                indices.push_back(i2);
                indices.push_back(i3);
            }
        }

        for (int z = 1; z < size; z++)
        {
            for (int x = 1; x < size; x++)
            {
                int i = z * (size + 1) + x;

                float hl = GetHeight((x - 1) * spacing, z * spacing);
                float hr = GetHeight((x + 1) * spacing, z * spacing);
                float hd = GetHeight(x * spacing, (z - 1) * spacing);
                float hu = GetHeight(x * spacing, (z + 1) * spacing);

                glm::vec3 normal = glm::normalize(glm::vec3(
                    hl - hr,
                    2.0f,
                    hd - hu));

                vertices[i].normal = normal;
            }
        }

        auto mesh = std::make_shared<Mesh>();
        mesh->SetName("Terrain");
        mesh->SetVertices(vertices);
        mesh->SetIndices(indices);

        return mesh;
    }

} // namespace ve
