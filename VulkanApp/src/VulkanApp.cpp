#include <VulkanEngine/VulkanEngine.hpp>
#include <VulkanEngine/Core/EntryPoint.hpp>

#include "RenderLayer.hpp"

class VulkanApp : public ve::Application
{
public:
    VulkanApp(const ve::ApplicationCreateInfo &createInfo) : ve::Application(createInfo)
    {
        PushLayer(new RenderLayer());
    }
};

ve::Application *ve::CreateApplication(ApplicationCommandLineArgs args)
{
    ve::ApplicationCreateInfo appInfo;
    appInfo.Name = "VulkanApp";
    appInfo.WorkingDirectory = "VulkanApp/";
    appInfo.CommandLineArgs = args;
    appInfo.WindowInfo = {.Title = "VulkanApp", .IconPath = "assets/icons/vulkan.png"};

    return new VulkanApp(appInfo);
}
