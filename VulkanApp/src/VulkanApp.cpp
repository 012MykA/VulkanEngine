#include <VulkanEngine/VulkanEngine.hpp>
#include <VulkanEngine/Core/EntryPoint.hpp>

#include "ExampleLayer.hpp"

class VulkanApp : public ve::Application
{
public:
    VulkanApp(const ve::ApplicationDesc &createInfo) : ve::Application(createInfo)
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
    ve::ApplicationDesc appInfo{
        .name = "VulkanApp",
        .workingDirectory = "./VulkanApp/",
        .commandLineArgs = args,
        .windowDesc{
            .title = "VulkanApp",
            .iconPath = "assets/icons/vulkan.png",
            .centered = true,
        },
    };

    return new VulkanApp(appInfo);
}
