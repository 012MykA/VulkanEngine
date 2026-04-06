#include "GlfwContext.hpp"
#include "VulkanEngine/Core/Log.hpp"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <stdexcept>

namespace ve
{
    namespace
    {
        void GlfwErrorCallback(int error, const char *description)
        {
            VE_CORE_ERROR("GLFW Error ({0}): {1}", error, description);
        }

    }

    ve::GlfwContext::GlfwContext()
    {
        glfwSetErrorCallback(GlfwErrorCallback);

        if (!glfwInit())
            throw std::runtime_error("Failed to initialize GLFW!");

        if (!glfwVulkanSupported())
            throw std::runtime_error("Vulkan is not supported by GLFW!");

        VE_CORE_TRACE("GLFW initialized");
    }

    GlfwContext::~GlfwContext()
    {
        VE_CORE_TRACE("GLFW terminated");
    }

} // namespace ve
