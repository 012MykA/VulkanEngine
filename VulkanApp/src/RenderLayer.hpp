#include <VulkanEngine/VulkanEngine.hpp>

class RenderLayer : public ve::Layer
{
public:
    RenderLayer();
    virtual ~RenderLayer() = default;

    virtual void OnAttach() override;
    virtual void OnDetach() override;
    virtual void OnUpdate(ve::Timestep ts) override;
    virtual void OnUIRender() override;
    virtual void OnEvent(ve::Event &event) override;

};
