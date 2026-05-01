#pragma once

#include "MaterialPBR.hpp"

#include <string>
#include <memory>

namespace ve
{
    class MaterialLoader
    {
    public:
        MaterialLoader() = default;
        ~MaterialLoader() = default;

        std::shared_ptr<MaterialPBR> LoadFromDirectory(const std::string &path);
    };

} // namespace ve
