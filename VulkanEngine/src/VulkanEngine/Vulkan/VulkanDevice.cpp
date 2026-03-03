#include "VulkanDevice.hpp"
#include "Validation.hpp"
#include "VulkanEngine/Core/Log.hpp"

#include <cassert>

namespace ve
{
    void VulkanPhysicalDevices::Init(VkInstance instance, VkSurfaceKHR surface)
    {
        uint32_t deviceCount = 0;
        validation::CheckVk(
            vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr),
            "enumerate physical devices (count)");

        if (deviceCount == 0)
            throw std::runtime_error("No GPUs found with Vulkan support!");

        VE_CORE_INFO("Physical Devices ({0}):", deviceCount);

        m_Devices.resize(deviceCount);

        std::vector<VkPhysicalDevice> devices(deviceCount);

        validation::CheckVk(
            vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data()),
            "enumerate physical devices (data)");

        for (uint32_t i = 0; i < deviceCount; i++)
        {
            VkPhysicalDevice physicalDevice = devices[i];
            m_Devices[i].physicalDevice = physicalDevice;

            vkGetPhysicalDeviceProperties(physicalDevice, &m_Devices[i].deviceProps);

            const auto &props = m_Devices[i].deviceProps;

            VE_CORE_INFO("Device [{0}]", i);
            VE_CORE_INFO("  Name: {0}", props.deviceName);

            uint32_t apiVersion = props.apiVersion;
            VE_CORE_INFO("  API Version: {0}.{1}.{2}.{3}",
                         VK_API_VERSION_VARIANT(apiVersion),
                         VK_API_VERSION_MAJOR(apiVersion),
                         VK_API_VERSION_MINOR(apiVersion),
                         VK_API_VERSION_PATCH(apiVersion));

            // === Queue families ===
            uint32_t queueFamiliesCount = 0;
            vkGetPhysicalDeviceQueueFamilyProperties(
                physicalDevice, &queueFamiliesCount, nullptr);

            VE_CORE_INFO("  Queue Families: {0}", queueFamiliesCount);

            m_Devices[i].queueFamilyProps.resize(queueFamiliesCount);
            m_Devices[i].queueSupportsPresent.resize(queueFamiliesCount);

            vkGetPhysicalDeviceQueueFamilyProperties(
                physicalDevice,
                &queueFamiliesCount,
                m_Devices[i].queueFamilyProps.data());

            for (uint32_t q = 0; q < queueFamiliesCount; q++)
            {
                validation::CheckVk(vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, q, surface, &(m_Devices[i].queueSupportsPresent[q])),
                                    "get physical device surface support!");
            }

            uint32_t formatCount = 0;
            validation::CheckVk(vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, nullptr),
                                "get physical device surface formats (count)!");
            assert(formatCount > 0);

            m_Devices[i].surfaceFormats.resize(formatCount);

            validation::CheckVk(vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, m_Devices[i].surfaceFormats.data()),
                                "get physical device surface formats (data)!");

            validation::CheckVk(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &(m_Devices[i].surfaceCapabilities)),
                                "get physical device surface capabilities!");

            uint32_t presentModes = 0;
            validation::CheckVk(vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModes, nullptr),
                                "get  physical device surface present modes (count)!");

            m_Devices[i].presentModes.resize(presentModes);

            validation::CheckVk(vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModes, m_Devices[i].presentModes.data()),
                                "get  physical device surface present modes (data)!");

            vkGetPhysicalDeviceMemoryProperties(physicalDevice, &(m_Devices[i].memoryProps));
        }
    }

    uint32_t VulkanPhysicalDevices::SelectDevice(VkQueueFlags requiredQueueType, bool supportsPresent)
    {
        for (uint32_t i = 0; i < m_Devices.size(); i++)
        {
            for (uint32_t j = 0; j < m_Devices[i].queueFamilyProps.size(); j++)
            {
                const VkQueueFamilyProperties &queueFamilyProps = m_Devices[i].queueFamilyProps[j];

                if ((queueFamilyProps.queueFlags & requiredQueueType) &&
                    (static_cast<bool>(m_Devices[i].queueSupportsPresent[j]) == supportsPresent))
                {
                    m_SelectedDeviceIndex = i;
                    int queueFamily = j;
                    VE_CORE_INFO("Using Graphics device {0} and queue family {1}", m_SelectedDeviceIndex, queueFamily);
                    return queueFamily;
                }
            }
        }

        VE_CORE_ERROR("Required queue type {0} and supports present {1} not found", requiredQueueType, supportsPresent);
        return 0;
    }

    const PhysicalDevice &VulkanPhysicalDevices::Selected() const
    {
        if (m_SelectedDeviceIndex < 0)
        {
            VE_CORE_ERROR("A physical device has not been selected yet!");
        }

        return m_Devices[m_SelectedDeviceIndex];
    }

} // namespace ve
