#pragma once

#include "Camera.hpp"
#include "VulkanEngine/Events/KeyEvent.hpp"

namespace ve
{
    class Window;

    struct FPSCameraDesc
    {
        float fov = 70.0f;

        float nearClip = 0.01f;
        float farClip = 1000.0f;

        float movementSpeed = 2.0f;
        float mouseSensitivity = 0.05f;
    };

    class FPSCamera : public Camera
    {
    public:
        FPSCamera(Window &window, const FPSCameraDesc &desc = {});
        virtual ~FPSCamera() override = default;

        virtual void OnUpdate(Timestep ts) override;
        virtual void OnEvent(Event &e) override;

    public:
        // clang-format off
        void SetYaw(float yaw) { m_Yaw = yaw; UpdateCameraVectors(); }
        void SetPitch(float pitch) { m_Pitch = pitch; UpdateCameraVectors(); }
        // clang-format on

    protected:
        virtual void UpdateViewMatrix() const override;
        virtual void UpdateProjectionMatrix() const override;

    private:
        void UpdateCameraVectors();

        bool OnKeyPressed(KeyPressedEvent &e);

    private:
        Window &m_Window;
        FPSCameraDesc m_Desc;

        float m_Yaw = -90.0f;
        float m_Pitch = 0.0f;

        glm::vec3 m_Front = {0.0f, 0.0f, -1.0f};
        glm::vec3 m_Up = {0.0f, 1.0f, 0.0f};
        glm::vec3 m_Right = {1.0f, 0.0f, 0.0f};
        glm::vec3 m_WorldUp = {0.0f, 1.0f, 0.0f};

    private:
        float m_LastMouseX = 0.0f, m_LastMouseY = 0.0f;
        bool m_FirstMouse = true;
    };

} // namespace ve
