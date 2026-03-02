#include <VulkanEngine/VulkanEngine.hpp>

class Sandbox3D : public ve::Layer
{
public:
    Sandbox3D();
    virtual ~Sandbox3D() = default;

    virtual void OnAttach() override;
    virtual void OnDetach() override;
    virtual void OnUpdate(ve::Timestep ts) override;
    virtual void OnUIRender() override;
    virtual void OnEvent(ve::Event &event) override;

};
