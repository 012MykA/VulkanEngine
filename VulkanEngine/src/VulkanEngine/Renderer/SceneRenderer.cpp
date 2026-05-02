#include "SceneRenderer.hpp"
#include "VulkanEngine/Renderer/RendererPBR.hpp"
#include "VulkanEngine/Core/Log.hpp"

namespace ve
{
    void SceneRenderer::AddScene(const gltf::Scene &scene, const glm::mat4 &transform)
    {
        m_Scenes.emplace_back(scene, transform);
    }

    void SceneRenderer::ClearScenes()
    {
        m_Scenes.clear();
    }

    void SceneRenderer::Draw(RendererPBR &renderer, const Camera &camera)
    {
        VkCommandBuffer cmd = renderer.GetCurrentCommandBuffer();
        uint32_t frameIndex = renderer.GetCurrentFrameIndex();
        Frustum frustum = Frustum::FromViewProjection(camera.GetViewProjection());

        renderer.BindPipeline(cmd);
        renderer.BindGlobalDescriptorSet(cmd, frameIndex);

        for (const auto &renderScene : m_Scenes)
        {
            for (int32_t nodeIdx : renderScene.scene.rootNodes)
            {
                DrawNode(renderer, cmd, renderScene.scene, nodeIdx, renderScene.transform, frustum);
            }
        }
    }

    void SceneRenderer::DrawNode(
        RendererPBR &renderer,
        VkCommandBuffer cmd,
        const gltf::Scene &scene,
        int32_t nodeIdx,
        const glm::mat4 &parentTransform,
        const Frustum &frustum)
    {
        const gltf::SceneNode &node = scene.nodes[nodeIdx];
        glm::mat4 worldTransform = parentTransform * node.localTransform;

        if (node.meshIndex >= 0)
        {
            const gltf::SceneMeshEntry &entry = scene.meshEntries[node.meshIndex];
            const auto &primitives = entry.mesh->GetPrimitives();

            entry.mesh->Bind(cmd);

            for (size_t i = 0; i < primitives.size(); i++)
            {
                const Primitive &prim = primitives[i];

                // Culling
                AABB primitiveWorldBounds = AABB::GetWorldAABB(prim.boundingBox, worldTransform);
                if (!frustum.TestAABB(primitiveWorldBounds))
                    continue;

                // Rendering
                int32_t matIdx = entry.materialIndices[i];
                const auto &material = scene.materials[matIdx];

                renderer.PushData(cmd, PushConstants{.model = worldTransform});
                renderer.BindMaterial(cmd, material);
                renderer.DrawIndexed(cmd, prim);
            }
        }

        for (int32_t childIdx : node.childIndices)
        {
            DrawNode(renderer, cmd, scene, childIdx, worldTransform, frustum);
        }
    }

} // namespace ve
