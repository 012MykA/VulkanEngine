cmake_minimum_required(VERSION 3.23)
project(VulkanEngine)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Vulkan REQUIRED)
add_subdirectory(vendor/glfw)
add_subdirectory(vendor/glm)

# Sources
file(GLOB_RECURSE ENGINE_SOURCES
    src/*.cpp
    vendor/stb_image/stb_image.c
    vendor/tinyobjloader/tiny_obj_loader.cc
)

add_library(${PROJECT_NAME} STATIC ${ENGINE_SOURCES})

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        Vulkan::Vulkan
        glfw
)

target_include_directories(${PROJECT_NAME}
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include/VulkanEngine
        ${CMAKE_CURRENT_SOURCE_DIR}/include/VulkanEngine/Core
        
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Assets
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Core
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Renderer
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Utils
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Vulkan

    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/vendor/stb_image
        ${CMAKE_CURRENT_SOURCE_DIR}/vendor/tinyobjloader
)
