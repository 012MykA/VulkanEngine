#include "Model.hpp"

#include <tiny_obj_loader.h>

#include <stdexcept>
#include <iostream>

namespace VE
{
    Model Model::LoadModel(const std::string &filename)
    {
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

                vertex.TexCoord = {
                    objAttrib.texcoords[2 * index.texcoord_index + 0],
                    1.0f - objAttrib.texcoords[2 * index.texcoord_index + 1],
                };

                vertex.Color = {1.0f, 1.0f, 1.0f};

                vertices.push_back(vertex);
                indices.push_back(indices.size());
            }
        }

        // Materials
        // std::vector<Material> materials;
        // for (const auto& material : objReader.GetMaterials())
        // {

        // }

        return Model(vertices, indices);
    }

    Model::Model(const std::vector<Vertex> &vertices, const std::vector<uint32_t> &indices)
        : m_Vertices(vertices), m_Indices(indices)
    {
    }
}
