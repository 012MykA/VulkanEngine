#include <VulkanEngine/VulkanEngine.hpp>

class Sandbox3D : public VE::Layer
{
public:
    Sandbox3D();
    virtual ~Sandbox3D() = default;

    virtual void OnAttach() override;
    virtual void OnDetach() override;
    virtual void OnUpdate(VE::Timestep ts) override;
    virtual void OnUIRender() override;
    virtual void OnEvent(VE::Event &event) override;

};
