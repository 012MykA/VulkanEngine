#pragma once

#include "VulkanEngine/Core/Layer.hpp"

#include <cstdint>
#include <functional>

namespace ve
{
    class FPSLayer : public Layer
    {
        using FPSCallback = std::function<void(uint32_t)>;

    public:
        FPSLayer(FPSCallback callback = nullptr)
            : Layer("FPSLayer"), m_Callback(callback) {}

        virtual void OnUpdate(ve::Timestep ts) override
        {
            m_TimeAccumulator += ts.GetSeconds();
            m_FrameCount++;

            if (m_TimeAccumulator >= 1.0f)
            {
                m_FPS = m_FrameCount;

                if (m_Callback)
                    m_Callback(m_FPS);

                m_FrameCount = 0;
                m_TimeAccumulator = 0.0f;
            }
        }

        uint32_t GetFPS() const { return m_FPS; }

    private:
        FPSCallback m_Callback;
        float m_TimeAccumulator = 0.0f;
        uint32_t m_FrameCount = 0.0f;
        uint32_t m_FPS = 0.0f;
    };

} // namespace ve
