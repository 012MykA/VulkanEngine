#pragma once

#include "VulkanAllocator.hpp"

#include <vulkan/vulkan.h>

#include <cstdint>
#include <cstring>
#include <span>

namespace ve
{
    class VulkanLogicalDevice;

    enum class BufferType : uint32_t
    {
        Vertex = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        Index = VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
        Uniform = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        Storage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
        Staging = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        IndirectDraw = VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT,
    };

    struct BufferDesc
    {
        VkDeviceSize size = 0;
        BufferType type = BufferType::Vertex;
        VkBufferUsageFlags extraUsage = 0;

        AllocationDesc allocation;
    };

    class VulkanBuffer
    {
    public:
        VulkanBuffer(const VulkanAllocator &allocator, const BufferDesc &desc);
        ~VulkanBuffer();

        // Copy
        VulkanBuffer(const VulkanBuffer &) = delete;
        VulkanBuffer &operator=(const VulkanBuffer &) = delete;

        // Move
        VulkanBuffer(VulkanBuffer &&other) noexcept;
        VulkanBuffer &operator=(VulkanBuffer &&other) noexcept;

    public:
        // Uploading from CPU
        void Upload(const void *data, VkDeviceSize size, VkDeviceSize offset = 0) const;

        template <typename T>
        void Upload(std::span<const T> data, VkDeviceSize offset = 0) const
        {
            Upload(data.data(), data.size_bytes(), offset);
        }

        template <typename T>
        void Upload(const T &value, VkDeviceSize offset = 0) const
        {
            Upload(&value, sizeof(T), offset);
        }

        void CopyTo(VkCommandBuffer cmd, const VulkanBuffer &dst,
                    VkDeviceSize srcOffset = 0,
                    VkDeviceSize dstOffset = 0,
                    VkDeviceSize size = VK_WHOLE_SIZE) const;

    public:
        // Getters
        VkBuffer GetVkHandle() const { return m_Buffer; }
        VkDeviceSize GetSize() const { return m_Desc.size; }
        void *GetMappedPtr() const { return m_Allocation.mappedPtr; }
        bool IsMapped() const { return m_Allocation.mappedPtr != nullptr; }

    private:
        void Destroy();

    private:
        const VulkanAllocator *m_Allocator = nullptr;
        VkBuffer m_Buffer = VK_NULL_HANDLE;
        Allocation m_Allocation = {};
        BufferDesc m_Desc = {};
    };

    // Staging buffer (CPU -> GPU)
    inline BufferDesc MakeStagingBufferDesc(VkDeviceSize size)
    {
        return BufferDesc{
            .size = size,
            .type = BufferType::Staging,
            .extraUsage = 0,
            .allocation = AllocationDesc{.cpuOnly = true},
        };
    }

    // GPU-only buffer
    inline BufferDesc MakeGPUBufferDesc(VkDeviceSize size, BufferType type)
    {
        return BufferDesc{
            .size = size,
            .type = type,
            .extraUsage = 0,
            .allocation = AllocationDesc{.gpuOnly = true},
        };
    }

    // CPU-to-GPU for UBO
    inline BufferDesc MakeUniformBufferDesc(VkDeviceSize size)
    {
        return BufferDesc{
            .size = size,
            .type = BufferType::Uniform,
            .extraUsage = 0,
            .allocation = AllocationDesc{.cpuToGpu = true, .persistentMap = true},
        };
    }

} // namespace ve
