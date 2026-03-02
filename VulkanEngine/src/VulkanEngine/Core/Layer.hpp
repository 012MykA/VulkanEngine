#pragma once

#include "VulkanEngine/Events/Event.hpp"
#include "VulkanEngine/Core/Timestep.hpp"

#include <string>

namespace VE
{
    class Layer
    {
    public:
        Layer(const std::string &debugName = "Untitled Layer");
        virtual ~Layer() = default;

        virtual void OnAttach() {}
        virtual void OnDetach() {}
        virtual void OnUpdate(Timestep ts) {}
        virtual void OnUIRender() {}
        virtual void OnEvent(Event &event) {}

        const std::string &GetName() const { return m_DebugName; }

    protected:
        std::string m_DebugName;
    };

} // namespace VE
