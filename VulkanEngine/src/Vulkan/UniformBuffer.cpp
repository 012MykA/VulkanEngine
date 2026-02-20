#include "UniformBuffer.hpp"
#include "Device.hpp"
#include "Buffer.hpp"
#include "DeviceMemory.hpp"

namespace VE
{
    UniformBuffer::UniformBuffer(const Device &device)
    {
        const auto bufferSize = sizeof(UniformBufferObject);

        m_Buffer = std::make_unique<Buffer>(device, bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
        m_Memory = std::make_unique<DeviceMemory>(
            m_Buffer->AllocateMemory(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                     VK_MEMORY_PROPERTY_HOST_COHERENT_BIT));
    }

    UniformBuffer::UniformBuffer(UniformBuffer &&other) noexcept
        : m_Buffer(other.m_Buffer.release()), m_Memory(other.m_Memory.release())
    {
    }

    UniformBuffer::~UniformBuffer() = default;

    void UniformBuffer::SetValue(const UniformBufferObject &ubo)
    {
        void *data = m_Memory->Map(0, sizeof(UniformBufferObject));
        std::memcpy(data, &ubo, sizeof(ubo));
        m_Memory->Unmap();
    }

}
