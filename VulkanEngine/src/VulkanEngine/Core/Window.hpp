#pragma once

#include "VulkanEngine/Events/Event.hpp"

#include <vulkan/vulkan.h>

#include <cstdint>
#include <string>
#include <filesystem>
#include <functional>
#include <vector>
#include <memory>

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
        using EventCallbackFn = std::function<void(Event &)>;

        virtual ~Window() = default;

        virtual void OnUpdate() = 0;

        virtual uint32_t GetWidth() const = 0;
        virtual uint32_t GetHeight() const = 0;
        virtual void *GetNativeWindow() const = 0;

        virtual void SetEventCallback(const EventCallbackFn &callback) = 0;

        // Vulkan
        virtual std::vector<const char *> GetRequiredVulkanExtensions() const = 0;
        virtual VkSurfaceKHR GetVulkanSurface(VkInstance instance) const = 0;

        static std::unique_ptr<Window> Create(const WindowCreateInfo &createInfo);
    };

} // namespace ve
