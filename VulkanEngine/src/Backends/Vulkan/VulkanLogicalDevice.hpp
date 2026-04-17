#pragma once

#include "VulkanPhysicalDevice.hpp"

#include <vulkan/vulkan.h>

#include <vector>

namespace ve
{
    struct LogicalDeviceDesc
    {
        std::vector<const char *> additionalExtensions;

        VkPhysicalDeviceFeatures enabledFeatures{};
    };

    struct DeviceQueues
    {
        VkQueue graphics = VK_NULL_HANDLE;
        VkQueue present = VK_NULL_HANDLE;
        VkQueue compute = VK_NULL_HANDLE;
        VkQueue transfer = VK_NULL_HANDLE;
    };

    class VulkanLogicalDevice
    {
    public:
        VulkanLogicalDevice(const VulkanPhysicalDevice &physicalDevice, const LogicalDeviceDesc &desc);
        ~VulkanLogicalDevice();

        VulkanLogicalDevice(const VulkanLogicalDevice &) = delete;
        VulkanLogicalDevice &operator=(const VulkanLogicalDevice &) = delete;

        void WaitIdle() const { vkDeviceWaitIdle(m_Device); }

    public:
        VkDevice GetVkHandle() const { return m_Device; }
        VkQueue GetGraphicsQueue() const { return m_Queues.graphics; }
        VkQueue GetPresentQueue() const { return m_Queues.present; }
        VkQueue GetComputeQueue() const { return m_Queues.compute; }
        VkQueue GetTransferQueue() const { return m_Queues.transfer; }

    private:
        void CreateDevice(const VulkanPhysicalDevice &physicalDevice, const LogicalDeviceDesc &desc);
        void RetrieveQueues(const QueueFamilyIndices &indices);

    private:
        VkDevice m_Device = VK_NULL_HANDLE;
        DeviceQueues m_Queues;
    };

} // namespace ve
