#include "Application.hpp"
#include "VulkanEngine/Core/Timestep.hpp"
#include "VulkanEngine/Utils/PlatformUtils.hpp"
#include "VulkanEngine/Vulkan/VulkanConfig.hpp"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <cassert>

namespace ve
{
    Application *Application::s_Instance = nullptr;

    Application::Application(const ApplicationCreateInfo &createInfo) : m_Info(createInfo)
    {
        assert(!s_Instance && "Application already exists!");
        s_Instance = this;

        std::filesystem::path workingDir(createInfo.WorkingDirectory);
        if (std::filesystem::exists(workingDir))
        {
            std::filesystem::current_path(workingDir);
        }

        m_Window = Window::Create(createInfo.WindowInfo);
        m_Window->SetEventCallback(VE_BIND_EVENT_FN(OnEvent));

        InitializeVulkan(createInfo.Name);
        CreateCommandBuffers();
    }

    void Application::Run()
    {
        while (m_Running)
        {
            float time = Time::GetTime();
            Timestep timestep = time - m_LastFrameTime;
            m_LastFrameTime = time;

            if (!m_Minimized)
            {
                for (Layer *layer : m_LayerStack)
                {
                    layer->OnUpdate(timestep);
                }
            }

            m_Window->OnUpdate();
        }
    }

    void Application::Close()
    {
        m_Running = false;
    }

    void Application::OnEvent(Event &e)
    {
        EventDispatcher dp(e);
        dp.Dispatch<WindowCloseEvent>(VE_BIND_EVENT_FN(OnWindowClose));
        dp.Dispatch<WindowResizeEvent>(VE_BIND_EVENT_FN(OnWindowResize));

        for (auto it = m_LayerStack.rbegin(); it != m_LayerStack.rend(); it++)
        {
            if (e.Handled)
                break;
            (*it)->OnEvent(e);
        }
    }

    void Application::PushLayer(Layer *layer)
    {
        m_LayerStack.PushLayer(layer);
    }

    void Application::PushOverlay(Layer *overlay)
    {
        m_LayerStack.PushOverlay(overlay);
    }

    bool Application::OnWindowClose(WindowCloseEvent &e)
    {
        m_Running = false;
        return true;
    }

    bool Application::OnWindowResize(WindowResizeEvent &e)
    {
        if (e.GetWidth() == 0 || e.GetHeight() == 0)
        {
            m_Minimized = true;
            return false;
        }

        // Renderer::OnWindowResize(e.GetWidth(), e.GetHeight());

        return false;
    }

    void Application::InitializeVulkan(const std::string &appName)
    {
        VulkanConfig config{};
        config.AppName = appName;
        config.AppVersion = VK_MAKE_VERSION(1, 0, 0);
        config.EngineName = "VulkanEngine";
        config.EngineVersion = VK_MAKE_VERSION(1, 0, 0);
        config.ApiVersion = VK_API_VERSION_1_3;
        config.InstanceExtensions = m_Window->GetRequiredVulkanExtensions();
#if defined(VE_DEBUG)
        config.EnableValidationLayers = true;
        config.ValidationLayers = {"VK_LAYER_KHRONOS_validation"};
        config.DebugConfig.EnableDebugMessenger = true;
        config.DebugConfig.MessageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                             VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
#endif

        m_VulkanCore.Init(config, static_cast<GLFWwindow *>(m_Window->GetNativeWindow()));
    }

    void Application::CreateCommandBuffers()
    {
        m_CommandBuffers.resize(MAX_FRAMES_IN_FLIGHT);
        m_VulkanCore.CreateCommandBuffers(MAX_FRAMES_IN_FLIGHT, m_CommandBuffers.data());
    }

} // namespace ve
