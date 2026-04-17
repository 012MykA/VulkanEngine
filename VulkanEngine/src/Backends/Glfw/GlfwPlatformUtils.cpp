#include "VulkanEngine/Utils/PlatformUtils.hpp"

#include <GLFW/glfw3.h>

namespace ve
{
    float Time::GetTime()
    {
        return static_cast<float>(glfwGetTime());
    }

} // namespace ve
