#include "CullingSystem.hpp"
#include "VulkanEngine/ECS/Components.hpp"

namespace ve
{
    CullingResult CullingSystem::Cull(const Frustum &frustum, Scene &scene)
    {
        CullingResult result;

        auto view = scene.m_Registry.view<TransformComponent, MeshComponent>();

        for (auto handle : view)
        {
            Entity entity(handle, &scene);
            result.totalTested++;

            const auto &transformComp = entity.GetComponent<TransformComponent>();
            const auto &meshComp = entity.GetComponent<MeshComponent>();

            if (!meshComp.mesh)
            {
                result.totalCulled++;
                continue;
            }

            const AABB &localBounds = meshComp.mesh->GetBoundingBox();
            const glm::mat4 &model = transformComp.GetTransform();

            AABB worldBounds;
            const glm::vec3 corners[8] = {
                {localBounds.min.x, localBounds.min.y, localBounds.min.z},
                {localBounds.max.x, localBounds.min.y, localBounds.min.z},
                {localBounds.min.x, localBounds.max.y, localBounds.min.z},
                {localBounds.max.x, localBounds.max.y, localBounds.min.z},
                {localBounds.min.x, localBounds.min.y, localBounds.max.z},
                {localBounds.max.x, localBounds.min.y, localBounds.max.z},
                {localBounds.min.x, localBounds.max.y, localBounds.max.z},
                {localBounds.max.x, localBounds.max.y, localBounds.max.z},
            };

            for (const auto &corner : corners)
            {
                glm::vec3 worldCorner = glm::vec3(model * glm::vec4(corner, 1.0f));
                worldBounds.Expand(worldCorner);
            }

            if (!frustum.TestAABB(worldBounds))
            {
                result.totalCulled++;
                continue;
            }

            result.visibleEntities.push_back(entity);
        }

        return result;
    }

    CullingResult CullingSystem::Cull(const glm::mat4 &vp, Scene &scene)
    {
        return Cull(Frustum::FromViewProjection(vp), scene);
    }

} // namespace ve
