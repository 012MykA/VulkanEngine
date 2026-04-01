#include "Camera.hpp"

#include "VulkanEngine/Core/Input.hpp"
#include "VulkanEngine/Core/Log.hpp"

namespace ve
{
    Camera::Camera(const CameraDesc &desc)
        : m_FOV(desc.FOV),
          m_AspectRatio(desc.aspectRatio),
          m_NearClip(desc.nearClip),
          m_FarClip(desc.farClip)
    {
        UpdateProjection();
        UpdateView();
    }

    void Camera::OnUpdate(Timestep ts)
    {
        const float speed = 5.0f;

        // WASD
        if (Input::IsKeyPressed(Key::W))
            m_Position += m_Front * speed * static_cast<float>(ts);

        if (Input::IsKeyPressed(Key::S))
            m_Position -= m_Front * speed * static_cast<float>(ts);

        if (Input::IsKeyPressed(Key::A))
            m_Position -= m_Right * speed * static_cast<float>(ts);

        if (Input::IsKeyPressed(Key::D))
            m_Position += m_Right * speed * static_cast<float>(ts);

        if (Input::IsKeyPressed(Key::Space))
            m_Position.y += speed * static_cast<float>(ts);

        if (Input::IsKeyPressed(Key::LeftControl))
            m_Position.y -= speed * static_cast<float>(ts);

        // Mouse
        if (Input::IsMouseButtonPressed(Mouse::ButtonRight))
        {
            auto [mouseX, mouseY] = Input::GetMousePosition();

            if (m_FirstMouse)
            {
                m_LastMouseX = mouseX;
                m_LastMouseY = mouseY;
                m_FirstMouse = false;
            }

            float xoffset = mouseX - m_LastMouseX;
            float yoffset = m_LastMouseY - mouseY;

            m_LastMouseX = mouseX;
            m_LastMouseY = mouseY;

            const float sensitivity = 0.1f;
            xoffset *= sensitivity;
            yoffset *= sensitivity;

            m_Yaw += xoffset;
            m_Pitch += yoffset;

            if (m_Pitch > 89.0f)
                m_Pitch = 89.0f;
            if (m_Pitch < -89.0f)
                m_Pitch = -89.0f;

            UpdateVectors();
        }

        UpdateView();
    }

    void Camera::UpdateProjection()
    {
        m_ProjectionMatrix = glm::perspective(
            glm::radians(m_FOV),
            m_AspectRatio,
            m_NearClip,
            m_FarClip);

        // Vulkan correction
        m_ProjectionMatrix[1][1] *= -1;
    }

    void Camera::UpdateView()
    {
        m_ViewMatrix = glm::lookAt(
            m_Position,
            m_Position + m_Front,
            m_Up);
    }

    void Camera::UpdateVectors()
    {
        glm::vec3 front;
        front.x = cos(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));
        front.y = sin(glm::radians(m_Pitch));
        front.z = sin(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));

        m_Front = glm::normalize(front);
        m_Right = glm::normalize(glm::cross(m_Front, glm::vec3(0.0f, 1.0f, 0.0f)));
        m_Up = glm::normalize(glm::cross(m_Right, m_Front));
    }

} // namespace ve
