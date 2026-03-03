#pragma once

#include "VulkanEngine/Events/Event.hpp"

#include <cstdint>
#include <string>
#include <filesystem>
#include <functional>

namespace ve
{
    struct WindowCreateInfo
    {
        std::string Title = "Untitled Window";
        std::filesystem::path IconPath;
        uint32_t Width = 800, Height = 600;
        bool Resizable = false;
        bool Fullscreen = false;
    };

    class Window
    {
    public:
        enum class Backend
        {
            None = 0,
            GLFW,
            Native
        };

        using EventCallbackFn = std::function<void(Event &)>;

        virtual ~Window() = default;

        virtual void OnUpdate() = 0;

        virtual uint32_t GetWidth() const = 0;
        virtual uint32_t GetHeight() const = 0;

        virtual void SetEventCallback(const EventCallbackFn &callback) = 0;

        static Scope<Window> Create(const WindowCreateInfo &createInfo, Window::Backend backend = Window::Backend::GLFW);
    };

} // namespace ve
