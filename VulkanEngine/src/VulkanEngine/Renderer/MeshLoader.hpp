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

        std::shared_ptr<Mesh> LoadGLB(const std::string &path);

        std::future<std::shared_ptr<Mesh>> LoadGLBAsync(const std::string &path);
    };

} // namespace ve
