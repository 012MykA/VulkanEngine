#include <VulkanEngine/VulkanEngine.hpp>
#include <VulkanEngine/Layers/FPSLayer.hpp>
#include <VulkanEngine/Core/EntryPoint.hpp>

#include "ExampleLayer.hpp"

class VulkanApp : public ve::Application
{
public:
    VulkanApp(const ve::ApplicationCreateInfo &createInfo) : ve::Application(createInfo)
    {
        PushLayer(new ExampleLayer());

        // clang-format off
        PushOverlay(new ve::FPSLayer([&](uint32_t fps) {
            VE_TRACE("Current FPS: {}", fps);
        }));
        // clang-format on
    }
};

ve::Application *ve::CreateApplication(ApplicationCommandLineArgs args)
{
    ve::ApplicationCreateInfo appInfo{
        .Name = "VulkanApp",
        .WorkingDirectory = "VulkanApp/",
        .CommandLineArgs = args,
        .WindowInfo{
            .Title = "VulkanApp",
            .IconPath = "assets/icons/vulkan.png",
            .Resizable = true,
        },
    };

    return new VulkanApp(appInfo);
}
