#include <VulkanEngine/VulkanEngine.hpp>

class ExampleLayer : public ve::Layer
{
public:
    ExampleLayer() : ve::Layer("ExampleLayer")
    {
    }

    virtual void OnEvent(ve::Event &e) override
    {
        ve::EventDispatcher dp(e);
        dp.Dispatch<ve::KeyPressedEvent>(VE_BIND_EVENT_FN(ExampleLayer::OnKeyPressed));
    }

private:
    bool OnKeyPressed(ve::KeyPressedEvent &e)
    {
        if (e.GetRepeatCount() > 0)
            return false;

        if (e.GetKeyCode() == ve::Key::F11)
        {
            auto& window = ve::Application::Get().GetWindow();
            VE_CORE_INFO("UwU");
        }
        
        return false;
    }
};
