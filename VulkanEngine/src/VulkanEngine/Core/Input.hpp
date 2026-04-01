#pragma once

#include "VulkanEngine/Core/KeyCodes.hpp"
#include "VulkanEngine/Core/MouseCodes.hpp"

#include <utility>

namespace ve
{
    class Input
    {
    public:
        static bool IsKeyPressed(int keycode);

        static bool IsMouseButtonPressed(int button);
        static std::pair<float, float> GetMousePosition();
        static float GetMouseX();
        static float GetMouseY();
    };

} // namespace ve
