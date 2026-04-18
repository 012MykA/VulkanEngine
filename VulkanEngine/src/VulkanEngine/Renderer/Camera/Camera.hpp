#pragma once

#include "VulkanEngine/Core/Timestep.hpp"
#include "VulkanEngine/Events/Event.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace ve
{
    class Camera
    {
    public:
        virtual ~Camera() = default;

        virtual void OnUpdate(Timestep ts) = 0;
        virtual void OnEvent(Event &) {}

    public:
        const glm::vec3 &GetPosition() const { return m_Position; }
        const glm::mat4 &GetView() const;
        const glm::mat4 &GetProjection() const;
        const glm::mat4 &GetViewProjection() const;

        void SetPosition(const glm::vec3 &position);
        void SetViewportSize(uint32_t width, uint32_t height);

    protected:
        virtual void UpdateViewMatrix() const = 0;
        virtual void UpdateProjectionMatrix() const = 0;

    protected:
        glm::vec3 m_Position = glm::vec3(0.0f);

        mutable glm::mat4 m_ViewMatrix = glm::mat4(1.0f);
        mutable glm::mat4 m_ProjectionMatrix = glm::mat4(1.0f);
        mutable glm::mat4 m_ViewProjectionMatrix = glm::mat4(1.0f);

        mutable bool m_IsViewDirty = true;
        mutable bool m_IsProjectionDirty = true;

        uint32_t m_ViewportWidth = 1280, m_ViewportHeight = 720;
        float m_AspectRatio = static_cast<float>(m_ViewportWidth) / static_cast<float>(m_ViewportHeight);
    };

} // namespace ve
