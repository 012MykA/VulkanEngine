#include "Window.hpp"

#if defined(VE_PLATFORM_WINDOWS)
    #include "Platform/Windows/WindowsWindow.hpp"
#elif defined(VE_PLATFORM_LINUX)
    #include "Platform/Linux/LinuxWindow.hpp"
#endif

namespace VE
{
    Scope<Window> VE::Window::Create(const WindowConfig &config)
    {
#if defined(VE_PLATFORM_WINDOWS)
        return CreateScope<WindowsWindow>(config);
#elif defined(VE_PLATFORM_LINUX)
        return CreateScope<LinuxWindow>(config);
#endif
    }

} // namespace VE
