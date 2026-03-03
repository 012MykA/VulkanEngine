cmake_minimum_required(VERSION 3.23)
project(VulkanEngine)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Vulkan REQUIRED)

add_subdirectory(vendor/spdlog)
add_subdirectory(vendor/glfw)

# Sources
file(GLOB_RECURSE ENGINE_CORE_SOURCES
    src/VulkanEngine/*.cpp
)

# Platform Sources
# Common
if(UNIX OR WIN32)
    file(GLOB_RECURSE PLATFORM_GLFW_SOURCES src/Platform/Glfw/*.cpp)
    list(APPEND PLATFORM_SOURCES ${PLATFORM_GLFW_SOURCES})
endif()

# Windows
if(WIN32)
    file(GLOB_RECURSE PLATFORM_WINDOWS_SOURCES src/Platform/Windows/*.cpp)
    list(APPEND PLATFORM_SOURCES ${PLATFORM_WINDOWS_SOURCES})
elseif(UNIX AND NOT APPLE) # Linux
    file(GLOB_RECURSE PLATFORM_LINUX_SOURCES src/Platform/Linux/*.cpp)
    list(APPEND PLATFORM_SOURCES ${PLATFORM_LINUX_SOURCES})
endif()
# End Platform Sources

set(STB_IMAGE_SOURCE vendor/stb_image/stb_image.c)

add_library(${PROJECT_NAME} STATIC
    ${ENGINE_CORE_SOURCES}
    ${PLATFORM_SOURCES}
    ${STB_IMAGE_SOURCE}
)

target_link_libraries(${PROJECT_NAME}
    PUBLIC
        Vulkan::Vulkan
    
    PRIVATE
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
        ${CMAKE_CURRENT_SOURCE_DIR}/vendor/stb_image
)
