#include <VulkanEngine/VulkanEngine.hpp>
#include <VulkanEngine/Core/EntryPoint.hpp>

#include "Sandbox3D.hpp"

class VulkanApp : public VE::Application
{
public:
    VulkanApp()
    {
        PushLayer(new Sandbox3D());
    }
};

VE::Application *VE::CreateApplication()
{
    return new VulkanApp();
}
