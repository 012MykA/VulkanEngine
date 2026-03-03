#include <VulkanEngine/VulkanEngine.hpp>
#include <VulkanEngine/Core/EntryPoint.hpp>

#include "RenderLayer.hpp"

class VulkanApp : public ve::Application
{
public:
    VulkanApp()
    {
        PushLayer(new RenderLayer());
    }
};

ve::Application *ve::CreateApplication()
{
    return new VulkanApp();
}
