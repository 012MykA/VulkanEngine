#pragma once

#include "VulkanEngine/Events/Event.hpp"
#include "KeyCodes.hpp"
#include "MouseCodes.hpp"

#include <vulkan/vulkan.h>

#include <cstdint>
#include <string>
#include <filesystem>
#include <functional>
#include <vector>
#include <memory>

namespace ve
{
    struct WindowDesc
    {
        std::string title = "Untitled Window";
        std::filesystem::path iconPath;
        uint32_t width = 1280, height = 720;
        uint32_t posX = 100, posY = 100;
        bool resizable = true;
        bool fullscreen = false;
        bool centered = false;
    };

    class Window
    {
    public:
        using EventCallbackFn = std::function<void(Event &)>;

        static std::unique_ptr<Window> Create(const WindowDesc &createInfo);

        virtual ~Window() = default;

        virtual void OnUpdate() = 0;

        // Vulkan
        virtual std::vector<const char *> GetRequiredVulkanExtensions() const = 0;
        virtual VkSurfaceKHR GetVulkanSurface(VkInstance instance) const = 0;

        // Input
        virtual bool IsKeyPressed(KeyCode keycode) const = 0;
        virtual bool IsMouseButtonPressed(MouseCode button) const = 0;
        virtual std::pair<float, float> GetMousePosition() const = 0;

        // Cursor
        virtual bool IsCursorLocked() const = 0;
        virtual void SetCursorLocked(bool locked) = 0;

    public: // Getters
        virtual uint32_t GetWidth() const = 0;
        virtual uint32_t GetHeight() const = 0;
        virtual void *GetNativeWindow() const = 0;

    public: // Setters
        virtual void SetEventCallback(const EventCallbackFn &callback) = 0;
    };

} // namespace ve
