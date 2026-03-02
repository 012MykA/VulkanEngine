#pragma once

#include "VulkanEngine/Core/Window.hpp"

namespace VE
{
    class LinuxWindow : public Window
    {
    public:
        LinuxWindow(const WindowConfig& config) {}

        virtual void OnUpdate() override {}

        virtual uint32_t GetWidth() const override {}
        virtual uint32_t GetHeight() const override {}

        virtual void SetEventCallback(const EventCallbackFn &callback) override {}

    private:
    };

} // namespace VE
