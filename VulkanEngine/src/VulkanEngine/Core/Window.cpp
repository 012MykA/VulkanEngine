#include "Window.hpp"
#include "VulkanEngine/Core/Base.hpp"

#if defined(VE_PLATFORM_WINDOWS) || defined(VE_PLATFORM_LINUX)
    #include "Platform/Glfw/GlfwWindowDriver.hpp"
#endif

#include <cassert>

namespace ve
{
    Scope<Window> ve::Window::Create(const WindowCreateInfo &createInfo)
    {
        return CreateScope<GlfwWindowDriver>(createInfo);
    }

} // namespace ve
