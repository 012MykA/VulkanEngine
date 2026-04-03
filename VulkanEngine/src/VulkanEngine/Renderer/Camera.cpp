#include "Camera.hpp"

#include "VulkanEngine/Core/Log.hpp"
#include "VulkanEngine/Core/Window.hpp"

namespace ve
{
    Camera::Camera(Window &window, const CameraDesc &desc)
        : m_Window(window),
          m_FOV(desc.FOV),
          m_AspectRatio(desc.aspectRatio),
          m_NearClip(desc.nearClip),
          m_FarClip(desc.farClip)
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

        const float speed = 2.0f;

        // WASD
        if (m_Window.IsKeyPressed(Key::W))
            m_Position += m_Front * speed * static_cast<float>(ts);

        if (m_Window.IsKeyPressed(Key::S))
            m_Position -= m_Front * speed * static_cast<float>(ts);

        if (m_Window.IsKeyPressed(Key::A))
            m_Position -= m_Right * speed * static_cast<float>(ts);

        if (m_Window.IsKeyPressed(Key::D))
            m_Position += m_Right * speed * static_cast<float>(ts);

        if (m_Window.IsKeyPressed(Key::Space))
            m_Position.y += speed * static_cast<float>(ts);

        if (m_Window.IsKeyPressed(Key::LeftControl))
            m_Position.y -= speed * static_cast<float>(ts);

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
