#pragma once

#include "VulkanEngine/Renderer/RendererPBR.hpp"
#include "VulkanEngine/Assets/GLTFScene.hpp"
#include "VulkanEngine/Renderer/Camera/Camera.hpp"
#include "VulkanEngine/Renderer/Frustum.hpp"

#include <memory>
#include <vector>
#include <queue>

namespace ve
{
    class RendererPBR;

    class SceneRenderer
    {
    public:
        SceneRenderer() = default;
        ~SceneRenderer() = default;

        SceneRenderer(const SceneRenderer &) = delete;
        SceneRenderer &operator=(const SceneRenderer &) = delete;

        void AddScene(const gltf::Scene &scene);
        void ClearScenes();

        void Draw(RendererPBR &renderer, const Camera &camera);

    private:
        void DrawNode(RendererPBR &renderer,
                      VkCommandBuffer cmd,
                      const gltf::Scene &scene,
                      int32_t nodeIdx,
                      const glm::mat4 &parentTransform,
                      const Frustum &frustum);

    private:
        std::vector<gltf::Scene> m_Scenes;
    };

} // namespace ve
