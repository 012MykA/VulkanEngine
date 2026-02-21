#include "Model.hpp"
#include "Timer.hpp"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>
#include <tiny_obj_loader.h>

#include <stdexcept>
#include <iostream>
#include <unordered_map>

namespace std
{
    template <>
    struct hash<VE::Vertex>
    {
        size_t operator()(VE::Vertex const &vertex) const
        {
            return ((hash<glm::vec3>()(vertex.Position) ^ (hash<glm::vec3>()(vertex.Color) << 1)) >> 1) ^
                   (hash<glm::vec2>()(vertex.TexCoord) << 1);
        }
    };

}

namespace VE
{
    Model Model::LoadModel(const std::string &filename)
    {
        std::cout << "Loading model: " << filename << "... " << std::flush;
        Timer timer;

        tinyobj::ObjReader objReader;

        if (!objReader.ParseFromFile(filename))
            throw std::runtime_error("failed to load model: '" + filename + "'\n" + objReader.Error());

        if (!objReader.Warning().empty())
        {
            std::cout << "\nWARNING: " << objReader.Warning() << std::endl;
        }

        // Attrib
        const auto &objAttrib = objReader.GetAttrib();

        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;
        std::unordered_map<Vertex, uint32_t> uniqueVertices{};

        for (const auto &shape : objReader.GetShapes())
        {
            const auto &mesh = shape.mesh;

            for (const auto &index : mesh.indices)
            {
                Vertex vertex{};

                vertex.Position = {
                    objAttrib.vertices[3 * index.vertex_index + 0],
                    objAttrib.vertices[3 * index.vertex_index + 1],
                    objAttrib.vertices[3 * index.vertex_index + 2],
                };

                if (!objAttrib.texcoords.empty())
                {
                    vertex.TexCoord = {
                        objAttrib.texcoords[2 * index.texcoord_index + 0],
                        1.0f - objAttrib.texcoords[2 * index.texcoord_index + 1],
                    };
                }

                vertex.Color = {1.0f, 1.0f, 1.0f};

                if (!uniqueVertices.contains(vertex))
                {
                    uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
                    vertices.push_back(vertex);
                }

                indices.push_back(uniqueVertices[vertex]);
            }
        }

        // Materials
        // std::vector<Material> materials;
        // for (const auto& material : objReader.GetMaterials())
        // {

        // }

        const double elapsed = timer.ElapsedMilliseconds();
        std::cout << "Loaded model: " << filename;
        std::cout << "\nVertices Count: " << vertices.size();
        std::cout << "\nIndices Count: " << indices.size();
        std::cout << "\nElapsed: " << elapsed << "ms" << std::endl;

        return Model(vertices, indices);
    }

    Model::Model(const std::vector<Vertex> &vertices, const std::vector<uint32_t> &indices)
        : m_Vertices(vertices), m_Indices(indices)
    {
    }
}
