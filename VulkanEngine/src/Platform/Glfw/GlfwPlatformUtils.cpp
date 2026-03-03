#include "VulkanEngine/Core/Base.hpp"

#ifdef VE_PLATFORM_WINDOWS

#include "VulkanEngine/Utils/PlatformUtils.hpp"

#include <GLFW/glfw3.h>

namespace ve
{
    float Time::GetTime()
    {
        return static_cast<float>(glfwGetTime());
    }

} // namespace ve

#endif // VE_PLATFORM_WINDOWS
