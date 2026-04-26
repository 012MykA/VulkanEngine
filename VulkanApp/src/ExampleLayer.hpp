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
    void SetupScene();

    bool OnWindowResize(ve::WindowResizeEvent &e);

private:
    void SetupMeshes();
    void SetupMaterials();
    void SetupObjects();

private:
    std::unique_ptr<ve::RendererPBR> m_Renderer;
    std::unique_ptr<ve::FPSCamera> m_Camera;
    std::unique_ptr<ve::Scene> m_Scene;

    std::shared_ptr<ve::Mesh> m_SphereMesh;
    std::shared_ptr<ve::Mesh> m_CubeMesh;
    std::shared_ptr<ve::Mesh> m_GLTFSceneMesh;

    std::shared_ptr<ve::MaterialPBR> m_DefaultMaterial;

    std::shared_ptr<ve::MaterialPBR> m_PurpleNeon;
    std::shared_ptr<ve::MaterialPBR> m_BlueNeon;
};
