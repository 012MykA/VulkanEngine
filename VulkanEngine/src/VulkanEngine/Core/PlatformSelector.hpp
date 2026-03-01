// Platform detection
#if defined(_WIN32)

    #if defined(_WIN64)
        #define VE_PLATFORM_WINDOWS
    #else
        #error "Windows x86 is not supported!"
    #endif

#elif defined(__ANDROID__)
    #error "Android platform is not supported!"
    #define VE_PLATFORM_ANDROID

#elif defined(__linux__)
    #error "Linux platform is not supported!"
    #define VE_PLATFORM_LINUX

#elif defined(__APPLE__) || defined(__MACH__)
    #error "Apple platform is not supported!"

    #include <TargetConditionals.h>
    #define VE_PLATFORM_APPLE

    #if TARGET_IPHONE_SIMULATOR
        #define VE_PLATFORM_IOS
        #define VE_PLATFORM_IOS_SIMULATOR
    #elif TARGET_OS_IPHONE
        #define VE_PLATFORM_IOS
    #elif TARGET_OS_MAC
        #define VE_PLATFORM_MACOS
    #else
        #error "Unknown Apple platform"
    #endif

#else
    #error "Unknown platform!"

#endif // End Platform detection
