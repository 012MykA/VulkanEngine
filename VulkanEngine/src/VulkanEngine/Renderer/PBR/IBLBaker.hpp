#pragma once

#include "VulkanEngine/Renderer/Texture.hpp"

#include <memory>

namespace ve
{
    class VulkanAllocator;
    class VulkanLogicalDevice;
    class VulkanImmediateSubmit;

    class IBLBaker
    {
    public:
        struct Result
        {
            std::shared_ptr<Texture> irradianceMap;  // Cubemap 32x32
            std::shared_ptr<Texture> prefilteredMap; // Cubemap 256x256 N mips
            std::shared_ptr<Texture> brdfLUT;        // 2D 512x512
        };

        static Result Bake(
            const Texture &envMap,
            const VulkanAllocator &allocator,
            const VulkanLogicalDevice &device,
            const VulkanImmediateSubmit &submit);
    };

} // namespace ve
