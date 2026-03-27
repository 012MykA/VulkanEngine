cmake_minimum_required(VERSION 3.23)
project(VulkanEngine)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Vendor
find_package(Vulkan REQUIRED)

add_subdirectory(vendor/spdlog)
add_subdirectory(vendor/glfw)
add_subdirectory(vendor/vma)

# Sources
file(GLOB_RECURSE ENGINE_CORE_SOURCES
    src/VulkanEngine/*.cpp
)

file(GLOB_RECURSE PLATFORM_GLFW_SOURCES src/Platform/Glfw/*.cpp)
list(APPEND PLATFORM_SOURCES ${PLATFORM_GLFW_SOURCES})

set(STB_IMAGE_SOURCE vendor/stb/src/stb_image.c)

add_library(${PROJECT_NAME} STATIC
    ${ENGINE_CORE_SOURCES}
    ${PLATFORM_SOURCES}
    ${STB_IMAGE_SOURCE}
)

target_link_libraries(${PROJECT_NAME}
    PUBLIC
        Vulkan::Vulkan
        GPUOpen::VulkanMemoryAllocator
        glfw
)

if (MSVC)
    target_compile_options(${PROJECT_NAME} PUBLIC /W4 /utf-8) # for spdlog
endif()

target_include_directories(${PROJECT_NAME}
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/src

        ${CMAKE_CURRENT_SOURCE_DIR}/vendor/spdlog/include
        ${CMAKE_CURRENT_SOURCE_DIR}/vendor/stb/include
)
