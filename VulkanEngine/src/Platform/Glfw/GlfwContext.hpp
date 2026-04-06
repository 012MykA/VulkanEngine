#pragma once

namespace ve
{
    // Manages glfwInit / glfwTerminate lifetime.
    // Must be created once before any GlfwWindowDriver and destroyed after all of them.
    class GlfwContext final
    {
    public:
        GlfwContext();
        ~GlfwContext();

        GlfwContext(const GlfwContext &) = delete;
        GlfwContext &operator=(const GlfwContext &) = delete;
    };

} // namespace ve
