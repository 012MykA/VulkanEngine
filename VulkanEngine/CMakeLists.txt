cmake_minimum_required(VERSION 3.23)
project(VulkanEngine)

option(VE_DIST_MODE "Build in Distribution mode (Windows App without console)" OFF)

if(VE_DIST_MODE AND NOT WIN32)
    message(FATAL_ERROR "VE_DIST_MODE is only supported on Windows!")
endif()

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

# Required packages
find_package(Vulkan REQUIRED)
find_package(VulkanMemoryAllocator CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(EnTT CONFIG REQUIRED)

find_path(TINYGLTF_INCLUDE_DIRS "tiny_gltf.h" REQUIRED)

set(STB_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/vendor/stb/src/stb_image.c
)

# Sources
file(GLOB_RECURSE ENGINE_CORE_SOURCES src/VulkanEngine/*.cpp)
file(GLOB_RECURSE BACKENDS_SOURCES src/Backends/*.cpp)

set(ALL_SOURCES
    ${ENGINE_CORE_SOURCES}
    ${BACKENDS_SOURCES}
    ${STB_SOURCES}
)

# Engine static lib
add_library(${PROJECT_NAME} STATIC ${ALL_SOURCES})

target_compile_definitions(${PROJECT_NAME} PUBLIC
    ${VE_PLATFORM_DEF}

    # Debug / Release
    $<$<CONFIG:Debug>:VE_DEBUG>
    $<$<NOT:$<CONFIG:Debug>>:VE_RELEASE>
)

if(VE_DIST_MODE)
    target_compile_definitions(${PROJECT_NAME} PUBLIC VE_DIST)
endif()

# Compiler settings
if(MSVC)
    target_compile_options(${PROJECT_NAME} PUBLIC /W4 /utf-8) # for spdlog
endif()

# Linking
target_include_directories(${PROJECT_NAME}
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include/
    ${CMAKE_CURRENT_SOURCE_DIR}/src/
    ${CMAKE_CURRENT_SOURCE_DIR}/vendor/stb/include/
    ${TINYGLTF_INCLUDE_DIRS}
)

target_link_libraries(${PROJECT_NAME}
    PUBLIC
    Vulkan::Vulkan
    GPUOpen::VulkanMemoryAllocator
    glfw
    spdlog::spdlog
    glm::glm
    EnTT::EnTT
)
