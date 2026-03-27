#pragma once

#include "VulkanPhysicalDevice.hpp"

#include <vulkan/vulkan.h>

#include <cstdint>
#include <vector>

namespace ve
{
    class VulkanLogicalDevice;

    enum class CommandPoolType
    {
        Graphics,
        Compute,
        Transfer
    };

    struct CommandPoolDesc
    {
        CommandPoolType type = CommandPoolType::Graphics;
        bool transient = false; // for ImmediateSubmit
        bool resetBuffer = true;
    };

    class VulkanCommandPool
    {
    public:
        VulkanCommandPool(const VulkanLogicalDevice &logicalDevice,
                          const VulkanPhysicalDevice &physicalDevice,
                          const CommandPoolDesc &desc);
        ~VulkanCommandPool();

        VulkanCommandPool(const VulkanCommandPool &) = delete;
        VulkanCommandPool &operator=(const VulkanCommandPool &) = delete;

        VkCommandPool GetVkHandle() const { return m_CommandPool; }

    public:
        // Memory allocation
        std::vector<VkCommandBuffer> AllocateBuffers(uint32_t count, VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY) const;
        VkCommandBuffer AllocateBuffer(VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY) const;

        void FreeBuffers(std::vector<VkCommandBuffer> &buffers) const;
        void FreeBuffer(VkCommandBuffer &buffer) const;

        void Reset() const;

    private:
        uint32_t GetQueueFamilyIndex(const VulkanPhysicalDevice &physicalDevice, CommandPoolType type) const;

    private:
        VkCommandPool m_CommandPool = VK_NULL_HANDLE;
        VkDevice m_Device = VK_NULL_HANDLE;
    };

} // namespace ve
