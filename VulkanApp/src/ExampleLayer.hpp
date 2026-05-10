#include <VulkanEngine/VulkanEngine.hpp>

class ExampleLayer : public ve::Layer
{
public:
    ExampleLayer();
    virtual ~ExampleLayer() override;

    virtual void OnAttach() override;
    virtual void OnUpdate(ve::Timestep ts) override;
    virtual void OnEvent(ve::Event &e) override;

private:
    bool OnWindowResize(ve::WindowResizeEvent &e);
    bool OnKeyPressed(ve::KeyPressedEvent &e);

private:
    void BuildLightTestScene();

private:
    std::unique_ptr<ve::RendererPBR> m_Renderer;
    std::unique_ptr<ve::FPSCamera> m_Camera;

    struct PendingScene
    {
        std::future<ve::gltf::Scene> scene;
        glm::mat4 transform = glm::mat4(1.0f);
    };
    std::vector<PendingScene> m_LoadingScenes;
    ve::SceneRenderer m_SceneRenderer;
};
