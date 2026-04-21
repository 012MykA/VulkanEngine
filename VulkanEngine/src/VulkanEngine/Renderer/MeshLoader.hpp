#pragma once

#include "Mesh.hpp"

#include <string>
#include <memory>
#include <future>

namespace ve
{
    class MeshLoader
    {
    public:
        MeshLoader() = default;
        ~MeshLoader() = default;

        // Load glTF / GLB
        std::shared_ptr<Mesh> LoadGLTF(const std::string &path);

        // Load glTF / GLB (async)
        std::future<std::shared_ptr<Mesh>> LoadGLTFAsync(const std::string &path);
    };

} // namespace ve
