#include "VulkanEngine/Core/Base.hpp"
#if defined(VE_PLATFORM_WINDOWS) || defined(VE_PLATFORM_LINUX)

#include "VulkanEngine/Utils/PlatformUtils.hpp"

#include <GLFW/glfw3.h>

namespace ve
{
    float Time::GetTime()
    {
        return static_cast<float>(glfwGetTime());
    }

} // namespace ve

#endif // VE_PLATFORM_WINDOWS || VE_PLATFORM_LINUX
