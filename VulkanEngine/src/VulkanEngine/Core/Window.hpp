#pragma once

#include "VulkanEngine/Events/Event.hpp"

#include <cstdint>
#include <string>
#include <filesystem>
#include <functional>

namespace VE
{
    struct WindowConfig
    {
        std::string Title = "Vulkan Window";
        std::filesystem::path IconPath;
        uint32_t Width = 800, Height = 600;
        bool Resizable = true;
        bool Fullscreen = false;
    };

    class Window
    {
    public:
        using EventCallbackFn = std::function<void(Event &)>;

        virtual ~Window() = default;

        virtual void OnUpdate() = 0;

        virtual uint32_t GetWidth() const = 0;
        virtual uint32_t GetHeight() const = 0;

        virtual void SetEventCallback(const EventCallbackFn &callback) = 0;

        static Scope<Window> Create(const WindowConfig &config = WindowConfig());
    };

} // namespace VE
