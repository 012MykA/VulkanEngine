cmake_minimum_required(VERSION 3.23)
project(VulkanEngine)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Vulkan REQUIRED)

# Sources
file(GLOB_RECURSE ENGINE_SOURCES
    src/*.cpp
)

add_library(${PROJECT_NAME} STATIC ${ENGINE_SOURCES})

target_link_libraries(${PROJECT_NAME}
    PUBLIC
        Vulkan::Vulkan
)

target_include_directories(${PROJECT_NAME}
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)
