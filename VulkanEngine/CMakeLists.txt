cmake_minimum_required(VERSION 3.23)
project(VulkanEngine)

# Language
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Platform
if(WIN32)
    if(NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
        message(FATAL_ERROR "Windows x86 is not supported!")
    endif()
    set(VE_PLATFORM_DEF VE_PLATFORM_WINDOWS)
elseif(APPLE)
    message(FATAL_ERROR "Apple platform is not supported!")
elseif(ANDROID)
    message(FATAL_ERROR "Android platform is not supported!")
elseif(UNIX)
    set(VE_PLATFORM_DEF VE_PLATFORM_LINUX)
else()
    message(FATAL_ERROR "Unknown platform!")
endif()

# Vendor
find_package(Vulkan REQUIRED)

add_subdirectory(vendor/spdlog)
add_subdirectory(vendor/glfw)
add_subdirectory(vendor/vma)

# Sources
file(GLOB_RECURSE ENGINE_CORE_SOURCES src/VulkanEngine/*.cpp)
file(GLOB_RECURSE PLATFORM_GLFW_SOURCES src/Platform/Glfw/*.cpp)
set(STB_IMAGE_SOURCES vendor/stb/src/stb_image.c)

set(ALL_SOURCES
    ${ENGINE_CORE_SOURCES}
    ${PLATFORM_GLFW_SOURCES}
    ${STB_IMAGE_SOURCES}
)

# Engine static lib
add_library(${PROJECT_NAME} STATIC
    ${ALL_SOURCES}
)

target_compile_definitions(${PROJECT_NAME} PUBLIC
    ${VE_PLATFORM_DEF}

    # Debug / Release
    $<$<CONFIG:Debug>:VE_DEBUG>
    $<$<NOT:$<CONFIG:Debug>>:VE_RELEASE>
)

if(MSVC)
    target_compile_options(${PROJECT_NAME} PUBLIC /W4 /utf-8) # for spdlog
endif()

target_include_directories(${PROJECT_NAME}
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/vendor/spdlog/include
    ${CMAKE_CURRENT_SOURCE_DIR}/vendor/stb/include
)

target_link_libraries(${PROJECT_NAME}
    PUBLIC
    Vulkan::Vulkan
    GPUOpen::VulkanMemoryAllocator
    glfw
)
