#include <VulkanEngine/VulkanEngine.hpp>
#include <VulkanEngine/Core/EntryPoint.hpp>

#include "Sandbox3D.hpp"

class VulkanApp : public ve::Application
{
public:
    VulkanApp()
    {
        PushLayer(new Sandbox3D());
    }
};

ve::Application *ve::CreateApplication()
{
    return new VulkanApp();
}
