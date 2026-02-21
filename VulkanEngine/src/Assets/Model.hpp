#pragma once

#include "Vertex.hpp"

#include <tiny_obj_loader.h>

#include <string>
#include <vector>

namespace VE
{
    class Model final
    {
    public:
        static Model LoadModel(const std::string &filename) { return Model(); }

    private:
        std::vector<Vertex> m_Vertices;
        std::vector<uint32_t> m_Indices;
    };

}
