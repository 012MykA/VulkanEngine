#pragma once

#include "Vertex.hpp"

#include <string>
#include <vector>

namespace VE
{
    class Model final
    {
    public:
        static Model LoadModel(const std::string &filename);

        const std::vector<Vertex>& Vertices() const { return m_Vertices; }
        const std::vector<uint32_t>& Indices() const { return m_Indices; }

    private:
        Model() = default;
        Model(const std::vector<Vertex> &vertices, const std::vector<uint32_t> &indices);

        std::vector<Vertex> m_Vertices;
        std::vector<uint32_t> m_Indices;
    };

}
