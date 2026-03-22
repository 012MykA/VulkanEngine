#pragma once

#include <cstdint>

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
        
    };

} // namespace ve
