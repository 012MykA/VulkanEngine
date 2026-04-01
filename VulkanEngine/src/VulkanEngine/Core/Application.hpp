#pragma once

#include "VulkanEngine/Core/Window.hpp"
#include "VulkanEngine/Core/LayerStack.hpp"
#include "VulkanEngine/Events/Event.hpp"
#include "VulkanEngine/Events/ApplicationEvent.hpp"
#include "VulkanEngine/Renderer/Renderer.hpp"
#include "VulkanEngine/Renderer/Camera.hpp"

#include <string>
#include <cassert>
#include <memory>

namespace ve
{
    struct ApplicationCommandLineArgs
    {
        int Count = 0;
        char **Args = nullptr;

        const char *operator[](int index) const
        {
            assert(index < Count && "Index out of range!");
            return Args[index];
        }
    };

    struct ApplicationCreateInfo
    {
        std::string Name = "VulkanEngine App";
        std::string WorkingDirectory;
        ApplicationCommandLineArgs CommandLineArgs;
        WindowCreateInfo WindowInfo;
    };

    class Application
    {
    public:
        Application(const ApplicationCreateInfo &createInfo);
        virtual ~Application() {}

    public:
        void Run();
        void Close();

        void OnEvent(Event &e);

        void PushLayer(Layer *layer);
        void PushOverlay(Layer *overlay);

        static Application &Get() { return *s_Instance; }
        Window &GetWindow() { return *m_Window; }

    private:
        bool OnWindowClose(WindowCloseEvent &e);
        bool OnWindowResize(WindowResizeEvent &e);

    private:
        ApplicationCreateInfo m_Info;
        std::unique_ptr<Window> m_Window;
        
        std::unique_ptr<Renderer> m_Renderer;
        std::unique_ptr<Camera> m_Camera;

    private:        
        bool m_Running = true;
        bool m_Minimized = false;
        LayerStack m_LayerStack;
        float m_LastFrameTime = 0.0f;

    private:
        static Application *s_Instance;
    };

    // To be defined in a CLIENT
    Application *CreateApplication(ApplicationCommandLineArgs args);

} // namespace ve
