#include "VulkanEngine/Core/Input.hpp"
#include "VulkanEngine/Core/Application.hpp"

#include <GLFW/glfw3.h>

namespace ve
{
    // Key
    bool Input::IsKeyPressed(int keycode)
    {
        auto window = static_cast<GLFWwindow *>(ve::Application::Get().GetWindow().GetNativeWindow());
        auto state = glfwGetKey(window, keycode);
        return state == GLFW_PRESS || state == GLFW_REPEAT;
    }

    // Mouse
    bool Input::IsMouseButtonPressed(int button)
    {
        auto window = static_cast<GLFWwindow *>(Application::Get().GetWindow().GetNativeWindow());
        auto state = glfwGetMouseButton(window, button);
        return state == GLFW_PRESS;
    }

    std::pair<float, float> Input::GetMousePosition()
    {
        auto window = static_cast<GLFWwindow *>(Application::Get().GetWindow().GetNativeWindow());
        double xPos, yPos;
        glfwGetCursorPos(window, &xPos, &yPos);

        return {static_cast<float>(xPos), static_cast<float>(yPos)};
    }

    float Input::GetMouseX()
    {
        auto [xPos, yPos] = GetMousePosition();

        return xPos;
    }

    float Input::GetMouseY()
    {
        auto [xPos, yPos] = GetMousePosition();

        return yPos;
    }

    // Cursor
    void Input::SetCursorLocked(bool locked)
    {
        auto window = static_cast<GLFWwindow *>(Application::Get().GetWindow().GetNativeWindow());

        glfwSetInputMode(window, GLFW_CURSOR, locked ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
    }

    void Input::SetCursorVisible(bool visible)
    {
        auto window = static_cast<GLFWwindow *>(Application::Get().GetWindow().GetNativeWindow());

        glfwSetInputMode(window, GLFW_CURSOR, visible ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_HIDDEN);
    }

} // namespace ve
