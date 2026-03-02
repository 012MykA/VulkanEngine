#include "Sandbox3D.hpp"

Sandbox3D::Sandbox3D() : Layer("Sandbox3D")
{
}

void Sandbox3D::OnAttach()
{
    VE_INFO("Layer {0} OnAttach", m_DebugName);
}

void Sandbox3D::OnDetach()
{
    VE_INFO("Layer {0} OnDetach", m_DebugName);
}

void Sandbox3D::OnUpdate(VE::Timestep ts)
{
    VE_INFO("Layer {0} OnUpdate", m_DebugName);
}

void Sandbox3D::OnUIRender()
{
    VE_INFO("Layer {0} OnUIRender", m_DebugName);
}

void Sandbox3D::OnEvent(VE::Event &event)
{
    VE_INFO("Layer {0} OnEvent", m_DebugName);
}
