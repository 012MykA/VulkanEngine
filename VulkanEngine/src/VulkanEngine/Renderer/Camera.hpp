#pragma once

#include "VulkanEngine/Core/Timestep.hpp"
#include "VulkanEngine/Events/Event.hpp"
#include "VulkanEngine/Events/KeyEvent.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace ve
{
    class Window;

    struct CameraDesc
    {
        float FOV = 45.0f;
        float aspectRatio = 1.778f; // (width / height)
        float nearClip = 0.001f;
        float farClip = 1000.0f;
    };

    class Camera
    {
    public:
        Camera(Window &window, const CameraDesc &desc);
        ~Camera() = default;

        void OnUpdate(Timestep ts);
        void OnEvent(Event &e);
        void OnResize(float width, float height);

    public: // Getters
        const glm::mat4 &GetViewMatrix() const { return m_ViewMatrix; }
        const glm::mat4 &GetProjectionMatrix() const { return m_ProjectionMatrix; }

    private:
        void UpdateProjection();
        void UpdateView();
        void UpdateVectors();

        bool OnKeyPressed(KeyPressedEvent &e);

    private:
        Window &m_Window;

        glm::mat4 m_ViewMatrix = glm::mat4(1.0f);
        glm::mat4 m_ProjectionMatrix = glm::mat4(1.0f);

        glm::vec3 m_Position{0.0f, 0.0f, 3.0f};

        glm::vec3 m_Front{0.0f, 0.0f, -1.0f};
        glm::vec3 m_Up{0.0f, 1.0f, 0.0f};
        glm::vec3 m_Right{1.0f, 0.0f, 0.0f};

        float m_Yaw = -90.0f;
        float m_Pitch = 0.0f;

        float m_FOV = 45.0f;
        float m_AspectRatio = 1.778f;

        float m_NearClip = 0.001f;
        float m_FarClip = 1000.0f;

        float m_LastMouseX = 0.0f;
        float m_LastMouseY = 0.0f;
        bool m_FirstMouse = true;
    };

} // namespace ve
