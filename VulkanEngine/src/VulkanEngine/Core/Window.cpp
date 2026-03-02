#include "Window.hpp"
#include "VulkanEngine/Core/Base.hpp"

#if defined(VE_PLATFORM_WINDOWS)
    #include "Platform/Windows/WindowsWindow.hpp"
#endif

namespace ve
{
    Scope<Window> ve::Window::Create(const WindowConfig &config)
    {
#if defined(VE_PLATFORM_WINDOWS)
        return CreateScope<WindowsWindow>(config);
#else
        return nullptr;
#endif
    }

} // namespace ve
