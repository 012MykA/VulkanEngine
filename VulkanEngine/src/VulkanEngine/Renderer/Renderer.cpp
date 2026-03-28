#include "Renderer.hpp"

namespace ve
{
    Renderer::Renderer(const Window &window)
    {
        Init(window);
    }

    bool Renderer::BeginFrame()
    {
        return false;
    }

    void Renderer::EndFrame()
    {
    }

    void Renderer::Init(const Window &window)
    {
    }

    void Renderer::RecreateSwapchain(uint32_t width, uint32_t height)
    {
    }

} // namespace ve
