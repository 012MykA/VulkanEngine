#include "VulkanCommandPool.hpp"
#include "VulkanLogicalDevice.hpp"
#include "VulkanEngine/Core/Log.hpp"
#include "Debug/VulkanValidation.hpp"

namespace ve
{
    VulkanCommandPool::VulkanCommandPool(
        const VulkanLogicalDevice &logicalDevice,
        const VulkanPhysicalDevice &physicalDevice,
        const CommandPoolDesc &desc)
        : m_Device(logicalDevice.GetVkHandle())
    {
        uint32_t familyIndex = GetQueueFamilyIndex(physicalDevice, desc.type);

        VkCommandPoolCreateFlags flags = 0;
        if (desc.transient)
            flags |= VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
        if (desc.resetBuffer)
            flags |= VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

        VkCommandPoolCreateInfo createInfo{
            .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            .flags = flags,
            .queueFamilyIndex = familyIndex,
        };

        VkResult result = vkCreateCommandPool(m_Device, &createInfo, nullptr, &m_CommandPool);
        CHECK_VK_RESULT(result);

        const char *typeStr;
        if (desc.type == CommandPoolType::Graphics)
            typeStr = "Graphics";
        else if (desc.type == CommandPoolType::Compute)
            typeStr = "Compute";
        else
            typeStr = "Transfer";
        VE_CORE_TRACE("CommandPool created");
        VE_CORE_TRACE("  type: {}", typeStr);
        VE_CORE_TRACE("  family: {}", familyIndex);
    }

    VulkanCommandPool::~VulkanCommandPool()
    {
        if (m_CommandPool != VK_NULL_HANDLE)
            vkDestroyCommandPool(m_Device, m_CommandPool, nullptr);
    }

    std::vector<VkCommandBuffer> VulkanCommandPool::AllocateBuffers(uint32_t count, VkCommandBufferLevel level) const
    {
        VkCommandBufferAllocateInfo allocInfo{
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .commandPool = m_CommandPool,
            .level = level,
            .commandBufferCount = count,
        };

        std::vector<VkCommandBuffer> buffers(count);
        VkResult result = vkAllocateCommandBuffers(m_Device, &allocInfo, buffers.data());
        CHECK_VK_RESULT(result);

        return buffers;
    }

    VkCommandBuffer VulkanCommandPool::AllocateBuffer(VkCommandBufferLevel level) const
    {
        VkCommandBufferAllocateInfo allocInfo{
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .commandPool = m_CommandPool,
            .level = level,
            .commandBufferCount = 1,
        };

        VkCommandBuffer buffer;
        VkResult result = vkAllocateCommandBuffers(m_Device, &allocInfo, &buffer);
        CHECK_VK_RESULT(result);

        return buffer;
    }

    void VulkanCommandPool::FreeBuffers(std::vector<VkCommandBuffer> &buffers) const
    {
        if (!buffers.empty())
        {
            vkFreeCommandBuffers(m_Device, m_CommandPool, static_cast<uint32_t>(buffers.size()), buffers.data());
            buffers.clear();
        }
    }

    void VulkanCommandPool::FreeBuffer(VkCommandBuffer &buffer) const
    {
        if (buffer != VK_NULL_HANDLE)
        {
            vkFreeCommandBuffers(m_Device, m_CommandPool, 1, &buffer);
            buffer = VK_NULL_HANDLE;
        }
    }

    void VulkanCommandPool::Reset() const
    {
        VkResult result = vkResetCommandPool(m_Device, m_CommandPool, 0);
        CHECK_VK_RESULT(result);
    }

    uint32_t VulkanCommandPool::GetQueueFamilyIndex(const VulkanPhysicalDevice &physicalDevice, CommandPoolType type) const
    {
        const auto &indices = physicalDevice.GetQueueFamilies();

        switch (type)
        {
        case CommandPoolType::Graphics:
            return indices.graphicsFamily.value();
        case CommandPoolType::Compute:
            return indices.computeFamily.value();
        case CommandPoolType::Transfer:
            return indices.GetTransferFamily();
        default:
            throw std::invalid_argument("Unknown CommandPoolType");
        }
    }

} // namespace ve
