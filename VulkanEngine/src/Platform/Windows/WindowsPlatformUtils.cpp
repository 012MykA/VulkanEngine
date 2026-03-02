#include "VulkanEngine/Core/Base.hpp"

#ifdef VE_PLATFORM_WINDOWS

#include "VulkanEngine/Utils/PlatformUtils.hpp"

#include <GLFW/glfw3.h>

namespace VE
{
    float Time::GetTime()
    {
        return static_cast<float>(glfwGetTime());
    }

} // namespace VE

#endif // VE_PLATFORM_WINDOWS
