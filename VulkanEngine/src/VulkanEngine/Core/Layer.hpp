#pragma once

#include "VulkanEngine/Events/Event.hpp"
#include "VulkanEngine/Core/Timestep.hpp"

#include <string>

namespace ve
{
    class Layer
    {
    public:
        Layer(const std::string &debugName = "Untitled Layer");
        virtual ~Layer() = default;

        virtual void OnAttach() {}
        virtual void OnDetach() {}
        virtual void OnUpdate(ve::Timestep) {}
        virtual void OnUIRender() {}
        virtual void OnEvent(ve::Event &) {}

        const std::string &GetName() const { return m_DebugName; }

    protected:
        std::string m_DebugName;
    };

} // namespace ve
