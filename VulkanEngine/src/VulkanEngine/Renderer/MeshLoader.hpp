#pragma once

#include "Mesh.hpp"

#include <string>
#include <memory>

namespace ve
{
    class MeshLoader
    {
    public:
        MeshLoader() = default;
        ~MeshLoader() = default;

        std::shared_ptr<Mesh> LoadGLB(const std::string &path);        
    };

} // namespace ve
