#include <VulkanEngine/VulkanEngine.hpp>
#include <VulkanEngine/Core/EntryPoint.hpp>

class VulkanApp : public VE::Application
{
public:
};

VE::Application *VE::CreateApplication()
{
    return new VulkanApp();
}