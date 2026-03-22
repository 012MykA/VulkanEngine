#pragma once

#include <cstdint>
#include <filesystem>

namespace ve
{
    using AssetHandle = uint64_t;

    enum class AssetType
    {
        None = 0,
        Texture,
        Shader,
        Mesh,
    };

    class Asset
    {
    public:
        AssetHandle Handle;
        std::filesystem::path FilePath;

        virtual ~Asset() = default;
        virtual AssetType GetType() const = 0;
    };

} // namespace ve
