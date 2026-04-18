#include <VulkanEngine/VulkanEngine.hpp>

class ExampleLayer : public ve::Layer
{
public:
    ExampleLayer();
    ~ExampleLayer() override;

    virtual void OnAttach() override;
    virtual void OnUpdate(ve::Timestep ts) override;
    virtual void OnEvent(ve::Event &e) override;

private:
    void SetupObjects();

    bool OnWindowResize(ve::WindowResizeEvent &e);

private:
    std::unique_ptr<ve::RendererPBR> m_Renderer;
    std::unique_ptr<ve::FPSCamera> m_Camera;
    std::unique_ptr<ve::Scene> m_Scene;
};
