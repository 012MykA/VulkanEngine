#pragma once

#include <glm/glm.hpp>

#include <memory>

namespace VE
{
    class Device;
    class Buffer;
    class DeviceMemory;

    struct UniformBufferObject
    {
        glm::mat4 Model;
        glm::mat4 View;
        glm::mat4 Proj;
    };

    class UniformBuffer
    {
    public:
        explicit UniformBuffer(const Device &device);
        UniformBuffer(UniformBuffer &&other) noexcept;
        ~UniformBuffer();

        void SetValue(const UniformBufferObject &ubo);

        const Buffer &GetBuffer() const { return *m_Buffer; }

    private:
        std::unique_ptr<Buffer> m_Buffer;
        std::unique_ptr<DeviceMemory> m_Memory;
    };

}
