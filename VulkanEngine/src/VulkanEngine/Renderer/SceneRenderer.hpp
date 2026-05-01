#pragma once

#include "VulkanEngine/Renderer/RendererPBR.hpp"
#include "VulkanEngine/Renderer/GLTFLoader.hpp"
#include "VulkanEngine/Renderer/Camera/Camera.hpp"

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

        void AddScene(const GLTFScene &scene);
        void ClearScenes();

        void Draw(RendererPBR &renderer, const Camera &camera);

    private:
        void DrawNode(
            RendererPBR &renderer,
            const GLTFScene &scene,
            int32_t nodeIdx,
            const glm::mat4 &parentTransform);

    private:
        std::vector<GLTFScene> m_Scenes;
    };

} // namespace ve
