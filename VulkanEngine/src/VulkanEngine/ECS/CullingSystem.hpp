#pragma once

#include "VulkanEngine/Renderer/Frustum.hpp"
#include "VulkanEngine/ECS/Scene.hpp"
#include "VulkanEngine/ECS/Entity.hpp"

#include <glm/glm.hpp>

#include <vector>

namespace ve
{
    struct CullingResult
    {
        std::vector<Entity> visibleEntities;
        uint32_t totalTested = 0;
        uint32_t totalCulled = 0;
    };

    class CullingSystem
    {
    public:
        static CullingResult Cull(const Frustum &frustum, Scene &scene);

        static CullingResult Cull(const glm::mat4 &vp, Scene &scene);
    };

} // namespace ve
