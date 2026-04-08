#pragma once

#include "VulkanEngine/Core/Window.hpp"
#include "VulkanEngine/Core/LayerStack.hpp"
#include "VulkanEngine/Events/Event.hpp"
#include "VulkanEngine/Events/ApplicationEvent.hpp"
#include "VulkanEngine/Renderer/PBR/RendererPBR.hpp"
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

    struct ApplicationDesc
    {
        std::string name = "VulkanEngine App";
        std::string workingDirectory;
        ApplicationCommandLineArgs commandLineArgs;
        WindowDesc windowDesc;
    };

    class Application
    {
    public:
        Application(const ApplicationDesc &desc);
        virtual ~Application();

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
        ApplicationDesc m_Desc;
        std::unique_ptr<Window> m_Window;

        std::unique_ptr<RendererPBR> m_Renderer;
        std::unique_ptr<Camera> m_Camera;

    private:
        bool m_Running = true;
        bool m_Minimized = false;
        LayerStack m_LayerStack;
        float m_LastFrameTime = 0.0f;

    private:
        std::vector<RenderObject> m_Objects;

    private:
        static Application *s_Instance;
    };

    // To be defined in a CLIENT
    Application *CreateApplication(ApplicationCommandLineArgs args);

} // namespace ve
