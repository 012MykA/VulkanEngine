#pragma once

#include "VulkanEngine/Core/PlatformSelector.hpp"

#include <memory>

// Build Configuration
#if defined(VE_DIST)
    #define VE_RELEASE
#elif defined(NDEBUG)
    #define VE_RELEASE
#else
    #define VE_DEBUG
#endif // Build Configuration

// Debugbreak support
#if defined(VE_DEBUG)
    #if defined(VE_PLATFORM_WINDOWS)
        #define VE_DEBUGBREAK() __debugbreak()
    #elif defined(VE_PLATFORM_LINUX)
        #include <signal.h>
        #define VE_DEBUGBREAK() raise(SIGTRAP)
    #else
        #warning "Platdorm does not support debugbreak!"
        #define VE_DEBUGBREAK()
    #endif
#else
    #define VE_DEBUGBREAK()
#endif // Debugbreak support

namespace VE
{
    template <typename T>
    using Scope = std::unique_ptr<T>;
    template <typename T, typename... Args>
    constexpr Scope<T> CreateScope(Args &&...args)
    {
        return std::make_unique<T>(std::forward<Args>(args)...);
    }

    template <typename T>
    using Ref = std::shared_ptr<T>;
    template <typename T, typename... Args>
    constexpr Ref<T> CreateRef(Args &&...args)
    {
        return std::make_shared<T>(std::forward<Args>(args)...);
    }

} // namespace VE
