#include "Window.hpp"
#include "Backends/Glfw/GlfwWindowDriver.hpp"

namespace ve
{
    std::unique_ptr<Window> ve::Window::Create(const WindowDesc &desc)
    {
        return std::make_unique<GlfwWindowDriver>(desc);
    }

} // namespace ve
