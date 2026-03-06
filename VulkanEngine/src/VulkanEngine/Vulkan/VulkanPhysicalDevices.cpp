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
    }

} // namespace ve
