#include "Window.hpp"
#include "Platform/Glfw/GlfwWindowDriver.hpp"

namespace ve
{
    std::unique_ptr<Window> ve::Window::Create(const WindowCreateInfo &createInfo)
    {
        return std::make_unique<GlfwWindowDriver>(createInfo);
    }

} // namespace ve
