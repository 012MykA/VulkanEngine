#pragma once

#include "VulkanEngine/Core/LayerStack.hpp"
#include "VulkanEngine/Core/Window.hpp"
#include "VulkanEngine/Events/ApplicationEvent.hpp"

#include <functional>
#include <memory>
#include <string>

namespace ve
{
    class Renderer;

    class AppWindow
    {
    public:
        using CloseCallback = std::function<void(AppWindow *)>;

    public:
        AppWindow(const WindowDesc &desc, CloseCallback onClose);
        ~AppWindow();

        AppWindow(const AppWindow &) = delete;
        AppWindow &operator=(const AppWindow &) = delete;

    public:
        // Events
        void OnEvent(Event &e);

        // Layers
        void PushLayer(Layer *layer);
        void PushOverlay(Layer *overlay);

    public:
        void OnUpdate(Timestep ts);
        void RenderFrame();

        void Close() { m_Open = false; }

    public: // Getters
        bool IsOpen() const { return m_Open; }
        const std::string &GetTitle() const { return m_Title; }
        Window &GetWindow() const { return *m_Window; }
        Renderer &GetRenderer() const { return *m_Renderer; }

    private: // Event functions
        bool OnWindowClose(WindowCloseEvent &e);
        bool OnWindowResize(WindowResizeEvent &e);

    private:
        std::string m_Title; // TODO: remove in Release/Dist mode (used for logging)
        std::unique_ptr<Window> m_Window;
        std::unique_ptr<Renderer> m_Renderer;
        LayerStack m_LayerStack;

        bool m_Open = true;
        bool m_Minimized = false;

        CloseCallback m_OnClose;
    };

} // namespace ve
