#include "VulkanLogicalDevice.hpp"
#include "Debug/VulkanValidation.hpp"
#include "VulkanEngine/Core/Log.hpp"

#include <set>

namespace ve
{
    VulkanLogicalDevice::VulkanLogicalDevice(const VulkanPhysicalDevice &physicalDevice, const LogicalDeviceDesc &desc)
    {
        CreateDevice(physicalDevice, desc);
        RetrieveQueues(physicalDevice.GetQueueFamilies());
    }

    VulkanLogicalDevice::~VulkanLogicalDevice()
    {
        if (m_Device != VK_NULL_HANDLE)
            vkDestroyDevice(m_Device, nullptr);
    }

    void VulkanLogicalDevice::CreateDevice(const VulkanPhysicalDevice &physicalDevice, const LogicalDeviceDesc &desc)
    {
        const auto &indices = physicalDevice.GetQueueFamilies();

        // Queues
        std::set<uint32_t> uniqueFamilies = {
            indices.graphicsFamily.value(),
            indices.presentFamily.value(),
            indices.computeFamily.value(),
            indices.GetTransferFamily(),
        };

        const float queuePriority = 1.0f;
        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
        queueCreateInfos.reserve(uniqueFamilies.size());

        for (uint32_t family : uniqueFamilies)
        {
            VkDeviceQueueCreateInfo queueCreateInfo{
                .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                .queueFamilyIndex = family,
                .queueCount = 1,
                .pQueuePriorities = &queuePriority,
            };
            queueCreateInfos.push_back(queueCreateInfo);
        }

        // Extensions
        std::vector<const char *> extensions(
            std::begin(VulkanPhysicalDevice::k_RequiredExtensions),
            std::end(VulkanPhysicalDevice::k_RequiredExtensions));

        for (const char *ext : desc.additionalExtensions)
            extensions.push_back(ext);

        // Device
        VkDeviceCreateInfo createInfo{
            .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
            .pNext = nullptr,
            .queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size()),
            .pQueueCreateInfos = queueCreateInfos.data(),
            .enabledExtensionCount = static_cast<uint32_t>(extensions.size()),
            .ppEnabledExtensionNames = extensions.data(),
            .pEnabledFeatures = &desc.enabledFeatures,
        };

        VkResult result = vkCreateDevice(physicalDevice.GetVkHandle(), &createInfo, nullptr, &m_Device);
        CHECK_VK_RESULT(result);

        VE_CORE_TRACE("Logical device created");
    }

    void VulkanLogicalDevice::RetrieveQueues(const QueueFamilyIndices &indices)
    {
        vkGetDeviceQueue(m_Device, indices.graphicsFamily.value(), 0, &m_Queues.graphics);
        vkGetDeviceQueue(m_Device, indices.presentFamily.value(), 0, &m_Queues.present);
        vkGetDeviceQueue(m_Device, indices.computeFamily.value(), 0, &m_Queues.compute);
        vkGetDeviceQueue(m_Device, indices.GetTransferFamily(), 0, &m_Queues.transfer);

        VE_CORE_TRACE("Queue families:");
        VE_CORE_TRACE("  Graphics: {}", indices.graphicsFamily.value());
        VE_CORE_TRACE("  Present: {}", indices.presentFamily.value());
        VE_CORE_TRACE("  Compute: {}", indices.computeFamily.value());
        VE_CORE_TRACE("  Transfer: {}", indices.GetTransferFamily());
    }

} // namespace ve
