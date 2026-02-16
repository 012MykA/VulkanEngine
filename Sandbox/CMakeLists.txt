cmake_minimum_required(VERSION 3.23)
project(Sandbox)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB_RECURSE SANDBOX_SOURCES
    src/*.cpp
    src/*.hpp
)

add_executable(${PROJECT_NAME} ${SANDBOX_SOURCES})

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        VulkanEngine
)

set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)
