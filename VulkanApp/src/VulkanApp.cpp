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
    appInfo.CommandLineArgs = args;

    return new VulkanApp(appInfo);
}
