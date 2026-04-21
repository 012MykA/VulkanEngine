#include "MaterialLoader.hpp"

namespace ve
{
    std::shared_ptr<MaterialPBR> MaterialLoader::LoadFromDirectory(const std::string &path)
    {
        return std::make_shared<MaterialPBR>();
    }

} // namespace ve
