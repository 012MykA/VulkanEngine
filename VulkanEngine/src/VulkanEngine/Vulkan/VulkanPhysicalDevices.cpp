#include "VulkanPhysicalDevices.hpp"
#include "VulkanEngine/Core/Log.hpp"
#include "Debug/VulkanLogUtils.hpp"

#include <stdexcept>

namespace ve
{
    void VulkanPhysicalDevices::Init(VkInstance instance, VkSurfaceKHR surface)
    {
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

        if (deviceCount == 0)
            throw std::runtime_error("failed to find GPUs with Vulkan support!");

        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

        m_PhysicalDevices.reserve(deviceCount);

        for (const auto &device : devices)
        {
            PhysicalDevice physicalDevice;
            physicalDevice.Device = device;
            vkGetPhysicalDeviceProperties(device, &physicalDevice.Properties);
            vkGetPhysicalDeviceFeatures(device, &physicalDevice.Features);
            vkGetPhysicalDeviceMemoryProperties(device, &physicalDevice.MemoryProperties);

            m_PhysicalDevices.push_back(physicalDevice);
        }

        // Logging
        VE_CORE_TRACE("Found {0} physical device(s) with Vulkan support:", m_PhysicalDevices.size());
        for (const auto &device : m_PhysicalDevices)
        {
            VE_CORE_TRACE("\tName: {0}", device.Properties.deviceName);
            VE_CORE_TRACE("\tType: {0}", VulkanDeviceTypeToString(device.Properties.deviceType));
            VE_CORE_TRACE("\tAPI Version: {0}.{1}.{2}",
                          VK_VERSION_MAJOR(device.Properties.apiVersion),
                          VK_VERSION_MINOR(device.Properties.apiVersion),
                          VK_VERSION_PATCH(device.Properties.apiVersion));
        }
        // ---
    }

} // namespace ve
