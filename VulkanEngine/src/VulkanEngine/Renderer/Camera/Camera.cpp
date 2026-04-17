#include "Camera.hpp"

#include "VulkanEngine/Core/Log.hpp"
#include "VulkanEngine/Core/Window.hpp"

namespace ve
{
    Camera::Camera(Window &window, const CameraDesc &desc)
        : m_Window(window), m_Desc(desc), m_AspectRatio(static_cast<float>(m_Window.GetWidth()) / m_Window.GetHeight())
    {
        m_Window.SetCursorLocked(true);
        UpdateProjection();
        UpdateView();
    }

    void Camera::OnUpdate(Timestep ts)
    {
        if (!m_Window.IsCursorLocked())
        {
            m_FirstMouse = true;
            return;
        }

        const float dt = static_cast<float>(ts);

        // WASD
        const float speed = m_Desc.moveSpeed;

        if (m_Window.IsKeyPressed(Key::W))
            m_Desc.position += m_Front * speed * dt;

        if (m_Window.IsKeyPressed(Key::S))
            m_Desc.position -= m_Front * speed * dt;

        if (m_Window.IsKeyPressed(Key::A))
            m_Desc.position -= m_Right * speed * dt;

        if (m_Window.IsKeyPressed(Key::D))
            m_Desc.position += m_Right * speed * dt;

        if (m_Window.IsKeyPressed(Key::Space))
            m_Desc.position.y += speed * dt;

        if (m_Window.IsKeyPressed(Key::LeftControl))
            m_Desc.position.y -= speed * dt;

        // Mouse
        auto [mouseX, mouseY] = m_Window.GetMousePosition();

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

        const float sensitivity = m_Desc.mouseSensitivity;
        xoffset *= sensitivity;
        yoffset *= sensitivity;

        m_Desc.yaw += xoffset;
        m_Desc.pitch += yoffset;

        if (m_Desc.pitch > 89.9f)
            m_Desc.pitch = 89.9f;
        if (m_Desc.pitch < -89.9f)
            m_Desc.pitch = -89.9f;

        UpdateVectors();
        UpdateView();
    }

    void Camera::OnEvent(Event &e)
    {
        EventDispatcher dp(e);
        dp.Dispatch<KeyPressedEvent>(VE_BIND_EVENT_FN(Camera::OnKeyPressed));
    }

    void Camera::OnResize(float width, float height)
    {
        m_AspectRatio = width / height;
        UpdateProjection();
    }

    void Camera::UpdateProjection()
    {
        m_ProjectionMatrix = glm::perspective(
            glm::radians(m_Desc.FOV),
            m_AspectRatio,
            m_Desc.nearClip,
            m_Desc.farClip);

        // Vulkan correction
        m_ProjectionMatrix[1][1] *= -1;
    }

    void Camera::UpdateView()
    {
        m_ViewMatrix = glm::lookAt(
            m_Desc.position,
            m_Desc.position + m_Front,
            m_Up);
    }

    void Camera::UpdateVectors()
    {
        glm::vec3 front;
        front.x = cos(glm::radians(m_Desc.yaw)) * cos(glm::radians(m_Desc.pitch));
        front.y = sin(glm::radians(m_Desc.pitch));
        front.z = sin(glm::radians(m_Desc.yaw)) * cos(glm::radians(m_Desc.pitch));

        m_Front = glm::normalize(front);
        m_Right = glm::normalize(glm::cross(m_Front, glm::vec3(0.0f, 1.0f, 0.0f)));
        m_Up = glm::normalize(glm::cross(m_Right, m_Front));
    }

    bool Camera::OnKeyPressed(KeyPressedEvent &e)
    {
        if (e.GetRepeatCount() > 0)
            return false;

        if (e.GetKeyCode() == Key::Escape)
        {
            bool isLocked = m_Window.IsCursorLocked();

            m_Window.SetCursorLocked(!isLocked);

            if (!isLocked)
                m_FirstMouse = true;

            return true;
        }

        return false;
    }

} // namespace ve
