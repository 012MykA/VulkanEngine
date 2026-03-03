#include "Window.hpp"
#include "VulkanEngine/Core/Base.hpp"

#if defined(VE_PLATFORM_WINDOWS)
    #include "Platform/Windows/WindowsWindow.hpp"
#endif

namespace ve
{
    Scope<Window> ve::Window::Create(const WindowCreateInfo &createInfo)
    {
#if defined(VE_PLATFORM_WINDOWS)
        return CreateScope<WindowsWindow>(createInfo);
#else
        return nullptr;
#endif
    }

} // namespace ve
