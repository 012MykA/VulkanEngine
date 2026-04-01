#pragma once

#include "VulkanEngine/Core/KeyCodes.hpp"
#include "VulkanEngine/Core/MouseCodes.hpp"

#include <utility>

namespace ve
{
    class Input
    {
    public:
        // Key
        static bool IsKeyPressed(int keycode);

        // Mouse
        static bool IsMouseButtonPressed(int button);
        static std::pair<float, float> GetMousePosition();
        static float GetMouseX();
        static float GetMouseY();

        // Cursor
        static void SetCursorLocked(bool locked);
        static void SetCursorVisible(bool visible);
    };

} // namespace ve
