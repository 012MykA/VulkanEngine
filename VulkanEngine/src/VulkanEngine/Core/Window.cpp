#include "Window.hpp"
#include "VulkanEngine/Core/Base.hpp"

#if defined(VE_PLATFORM_WINDOWS) || defined(VE_PLATFORM_LINUX)
#include "Platform/Glfw/GlfwWindowDriver.hpp"
#endif

#if defined(VE_PLATFORM_WINDOWS)
#include "Platform/Windows/WindowsWindowDriver.hpp"
#elif defined(VE_PLATFORM_LINUX)
#include "Platform/Linux/LinuxWindowDriver.hpp"
#endif

#include <cassert>

namespace ve
{
    Scope<Window> ve::Window::Create(const WindowCreateInfo &createInfo, Window::Backend backend)
    {
        switch (backend)
        {
        case Backend::GLFW:
        {
#if defined(VE_PLATFORM_WINDOWS)
            return CreateScope<WindowsWindowDriver>(createInfo);
#else
            return nullptr;
#endif
        }
        case Backend::Native:
        {
#if defined(VE_PLATFORM_WINDOWS)
            return CreateScope<WindowsWindowDriver>(createInfo);
#elif defined(VE_PLATFORM_LINUX)
            return CreateScope<LinuxWindowDriver>(createInfo);
#else
            MYKA_CORE_ASSERT(false, "Native backend not supported on this platform!");
            return nullptr;
#endif
        }
        case Backend::None:
        default:
            assert(false && "Window backend None is not supported!");
            return nullptr;
        }
    }

} // namespace ve
