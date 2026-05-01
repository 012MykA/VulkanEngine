#include "SceneRenderer.hpp"
#include "VulkanEngine/Renderer/RendererPBR.hpp"

namespace ve
{
    void SceneRenderer::AddScene(const GLTFScene &scene)
    {
        m_Scenes.push_back(scene);
    }

    void SceneRenderer::ClearScenes()
    {
        m_Scenes.clear();
    }

    void SceneRenderer::Draw(RendererPBR &renderer, const Camera &camera)
    {
        for (const auto &scene : m_Scenes)
        {
            for (int32_t nodeIdx : scene.rootNodes)
            {
                DrawNode(renderer, scene, nodeIdx, glm::mat4(1.0f));
            }
        }
    }

    void SceneRenderer::DrawNode(
        RendererPBR &renderer,
        const GLTFScene &scene,
        int32_t nodeIdx,
        const glm::mat4 &parentTransform)
    {
        const SceneNode &node = scene.nodes[nodeIdx];

        glm::mat4 worldTransform = parentTransform * node.localTransform;

        if (node.meshIndex >= 0)
        {
            const SceneMeshEntry &entry = scene.meshEntries[node.meshIndex];

            for (size_t i = 0; i < entry.mesh->GetPrimitives().size(); i++)
            {
                int32_t matIdx = entry.materialIndices[i];
                const auto &material = scene.materials[matIdx];

                renderer.Submit(RenderObject{
                    .transform = worldTransform,
                    .mesh = entry.mesh,
                    .material = material,
                    .primitiveIndex = static_cast<uint32_t>(i),
                });
            }
        }

        for (int32_t childIdx : node.childIndices)
        {
            DrawNode(renderer, scene, childIdx, worldTransform);
        }
    }

} // namespace ve
