#pragma once

#include "VulkanEngine/Core/Base.hpp"
#include "VulkanEngine/Core/Window.hpp"
#include "VulkanEngine/Core/LayerStack.hpp"
#include "VulkanEngine/Events/Event.hpp"
#include "VulkanEngine/Events/ApplicationEvent.hpp"

#include <string>
#include <cassert>

// TODO: refactor
#include "VulkanEngine/Vulkan/VulkanCore.hpp"

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

    private:
        bool OnWindowClose(WindowCloseEvent &e);
        bool OnWindowResize(WindowResizeEvent &e);

    private:
        ApplicationCreateInfo m_Info;
        Scope<Window> m_Window;
        bool m_Running = true;
        bool m_Minimized = false;
        LayerStack m_LayerStack;
        float m_LastFrameTime = 0.0f;

    private: // TODO: refactor
        void InitializeVulkan(const std::string &appName);
        void CreateCommandBuffers();

        VulkanCore m_VulkanCore;
        const uint32_t MAX_FRAMES_IN_FLIGHT = 3;
        std::vector<VkCommandBuffer> m_CommandBuffers;

    private:
        static Application *s_Instance;
    };

    // To be defined in a CLIENT
    Application *CreateApplication(ApplicationCommandLineArgs args);

} // namespace ve
