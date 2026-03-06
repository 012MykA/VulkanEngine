#include "VulkanPhysicalDevices.hpp"
#include "VulkanEngine/Core/Log.hpp"
#include "Debug/VulkanLogUtils.hpp"

#include <stdexcept>
#include <set>
#include <string>

namespace ve
{
    void VulkanPhysicalDevices::Init(VkInstance instance)
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

    void VulkanPhysicalDevices::SelectDevice(VkSurfaceKHR surface, const PhysicalDeviceRequirements &requirements)
    {
        for (size_t i = 0; i < m_PhysicalDevices.size(); ++i)
        {
            if (IsDeviceSuitable(m_PhysicalDevices[i], surface, requirements))
            {
                m_SelectedDeviceIndex = static_cast<uint32_t>(i);
                VE_CORE_TRACE("Selected physical device: {0}", m_PhysicalDevices[i].Properties.deviceName);
                return;
            }
        }

        throw std::runtime_error("failed to find a suitable GPU!");
    }

    bool VulkanPhysicalDevices::IsDeviceSuitable(const PhysicalDevice &device,
                                                 VkSurfaceKHR surface,
                                                 const PhysicalDeviceRequirements &requirements) const
    {
        PhysicalDeviceQueueFamilyIndices indices = FindQueueIndices(device.Device, surface);
        if (requirements.RequiresGraphicsQueue && !indices.GraphicsFamily.has_value())
            return false;
        if (requirements.RequiresPresentQueue && !indices.PresentFamily.has_value())
            return false;

        if (!CheckDeviceExtensionSupport(device.Device, requirements.Extensions))
            return false;

        if (!device.Features.geometryShader)
            return false;

        if (device.Properties.deviceType != VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
            return false;

        return true;
    }

    PhysicalDeviceQueueFamilyIndices VulkanPhysicalDevices::FindQueueIndices(VkPhysicalDevice device, VkSurfaceKHR surface) const
    {
        PhysicalDeviceQueueFamilyIndices indices;

        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

        for (uint32_t i = 0; i < queueFamilyCount; ++i)
        {
            if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
                indices.GraphicsFamily = i;

            VkBool32 presentSupport = VK_FALSE;
            vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
            if (presentSupport)
                indices.PresentFamily = i;

            if (indices.IsComplete())
                break;
        }

        return indices;
    }

    bool VulkanPhysicalDevices::CheckDeviceExtensionSupport(VkPhysicalDevice device,
                                                            const std::vector<const char *> &requiredExtensions) const
    {
        uint32_t extensionCount;
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

        std::vector<VkExtensionProperties> availableExtensions(extensionCount);
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

        std::set<std::string> requiredSet(requiredExtensions.begin(), requiredExtensions.end());
        for (const auto &ext : availableExtensions)
        {
            requiredSet.erase(ext.extensionName);
        }

        return requiredSet.empty();
    }

    PhysicalDeviceQueueFamilyIndices VulkanPhysicalDevices::GetQueueIndices(VkSurfaceKHR surface) const
    {
        return FindQueueIndices(m_PhysicalDevices[m_SelectedDeviceIndex].Device, surface);
    }

} // namespace ve
