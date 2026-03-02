#include "Window.hpp"

#ifdef VE_PLATFORM_WINDOWS
    #include "Platform/Windows/WindowsWindow.hpp"
#elifdef VE_PLATFORM_LINUX
    #include "Platform/Linux/LinuxWindow.hpp"
#endif

namespace VE
{
    Scope<Window> VE::Window::Create(const WindowConfig &config)
    {
#ifdef VE_PLATFORM_WINDOWS
        return CreateScope<WindowsWindow>(config);
#elifdef VE_PLATFORM_LINUX
        return CreateScope<LinuxWindow>(config);
#else
        return nullptr;
#endif
    }

} // namespace VE
