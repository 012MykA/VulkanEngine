#include <VulkanEngine/VulkanEngine.hpp>
#include <VulkanEngine/Core/EntryPoint.hpp>

#include "ExampleLayer.hpp"

class VulkanApp : public ve::Application
{
public:
    VulkanApp(const ve::ApplicationCreateInfo &createInfo) : ve::Application(createInfo)
    {
        PushLayer(new ExampleLayer());
    }
};

ve::Application *ve::CreateApplication(ApplicationCommandLineArgs args)
{
    std::string workingDir = "VulkanApp/";
    if (args.Count > 1)
        workingDir = args[1];

    ve::ApplicationCreateInfo appInfo{
        .Name = "VulkanApp",
        .WorkingDirectory = workingDir,
        .CommandLineArgs = args,
        .WindowInfo{
            .Title = "VulkanApp",
            .IconPath = "assets/icons/vulkan.png",
            .Resizable = true,
        },
    };

    return new VulkanApp(appInfo);
}
