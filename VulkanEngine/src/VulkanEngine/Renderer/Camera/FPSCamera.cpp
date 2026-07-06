#include "FPSCamera.hpp"
#include "VulkanEngine/Core/Window.hpp"
#include "VulkanEngine/Core/Base.hpp" // for VE_BIND_EVENT_FN()

namespace ve
{
    FPSCamera::FPSCamera(Window &window, const FPSCameraDesc &desc)
        : m_Desc(desc), m_Window(window)
    {
        Camera::SetViewportSize(m_Window.GetWidth(), m_Window.GetHeight());
        m_Window.SetCursorLocked(true);

        auto [mouseX, mouseY] = m_Window.GetMousePosition();
        m_LastMouseX = mouseX;
        m_LastMouseY = mouseY;

        UpdateCameraVectors();
    }

    void FPSCamera::OnUpdate(Timestep ts)
    {
        if (!m_Window.IsCursorLocked())
        {
            m_FirstMouse = true;
            return;
        }

        float sprintMultiplier = m_Window.IsKeyPressed(Key::LeftShift) ? m_Desc.sprintMultiplier : 1.0f;
        float velocity = m_Desc.movementSpeed * ts.GetSeconds();
        float speed = velocity * sprintMultiplier;

        // clang-format off
        if (m_Window.IsKeyPressed(Key::W)) { m_Position += m_Front * speed; m_IsViewDirty = true; }
        if (m_Window.IsKeyPressed(Key::S)) { m_Position -= m_Front * speed; m_IsViewDirty = true; }
        if (m_Window.IsKeyPressed(Key::A)) { m_Position -= m_Right * speed; m_IsViewDirty = true; }
        if (m_Window.IsKeyPressed(Key::D)) { m_Position += m_Right * speed; m_IsViewDirty = true; }

        if (m_Window.IsKeyPressed(Key::E)) { m_Position += m_Up * speed; m_IsViewDirty = true; }
        if (m_Window.IsKeyPressed(Key::Q)) { m_Position -= m_Up * speed; m_IsViewDirty = true; }
        // clang-format on

        auto [mouseX, mouseY] = m_Window.GetMousePosition();

        if (m_FirstMouse)
        {
            m_LastMouseX = mouseX;
            m_LastMouseY = mouseY;
            m_FirstMouse = false;
        }

        float xOffset = mouseX - m_LastMouseX;
        float yOffset = m_LastMouseY - mouseY;

        m_LastMouseX = mouseX;
        m_LastMouseY = mouseY;

        if (xOffset != 0 || yOffset != 0)
        {
            float sensitivity = m_Desc.mouseSensitivity;

            m_Yaw += xOffset * sensitivity;
            m_Pitch += yOffset * sensitivity;

            // clang-format off
            if (m_Pitch > 89.9f) m_Pitch = 89.9f;
            if (m_Pitch < -89.9f) m_Pitch = -89.9f;
            // clang-format on

            UpdateCameraVectors();
            m_IsViewDirty = true;
        }
    }

    void FPSCamera::OnEvent(Event &e)
    {
        EventDispatcher dp(e);
        dp.Dispatch<KeyPressedEvent>(VE_BIND_EVENT_FN(FPSCamera::OnKeyPressed));
    }

    void FPSCamera::UpdateViewMatrix() const
    {
        m_ViewMatrix = glm::lookAt(m_Position, m_Position + m_Front, m_Up);
    }

    void FPSCamera::UpdateProjectionMatrix() const
    {
        m_ProjectionMatrix = glm::perspective(
            glm::radians(m_Desc.fov),
            Camera::m_AspectRatio,
            m_Desc.nearClip,
            m_Desc.farClip);

        // Invert Y for Vulkan
        m_ProjectionMatrix[1][1] *= -1;
    }

    void FPSCamera::UpdateCameraVectors()
    {
        glm::vec3 front;
        front.x = cos(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));
        front.y = sin(glm::radians(m_Pitch));
        front.z = sin(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));

        m_Front = glm::normalize(front);
        m_Right = glm::normalize(glm::cross(m_Front, m_WorldUp));
        m_Up = glm::normalize(glm::cross(m_Right, m_Front));
    }

    bool FPSCamera::OnKeyPressed(KeyPressedEvent &e)
    {
        bool isRepeat = e.GetRepeatCount() > 0;

        if (e.GetKeyCode() == Key::Escape && !isRepeat)
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
