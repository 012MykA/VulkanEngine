#include "VulkanEngine/Core/Input.hpp"
#include "VulkanEngine/Core/Application.hpp"

#include <GLFW/glfw3.h>

namespace ve
{
    bool Input::IsKeyPressed(int keycode)
    {
        auto window = static_cast<GLFWwindow *>(ve::Application::Get().GetWindow().GetNativeWindow());
        auto state = glfwGetKey(window, keycode);
        return state == GLFW_PRESS || state == GLFW_REPEAT;
    }

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

} // namespace ve
