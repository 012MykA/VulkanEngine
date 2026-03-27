#include "VulkanPhysicalDevice.hpp"
#include "Debug/VulkanValidation.hpp"
#include "VulkanEngine/Core/Log.hpp"

#include <set>
#include <string>

namespace ve
{
    VulkanPhysicalDevice::VulkanPhysicalDevice(VkInstance instance, VkSurfaceKHR surface)
        : m_Surface(surface)
    {
        PickPhysicalDevice(instance, surface);
    }

    SwapchainSupportDetails VulkanPhysicalDevice::QuerySwapchainSupport() const
    {
        SwapchainSupportDetails details;

        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_PhysicalDevice, m_Surface, &details.capabilities);

        uint32_t formatCount = 0;
        vkGetPhysicalDeviceSurfaceFormatsKHR(m_PhysicalDevice, m_Surface, &formatCount, nullptr);
        if (formatCount > 0)
        {
            details.formats.resize(formatCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(m_PhysicalDevice, m_Surface, &formatCount, details.formats.data());
        }

        uint32_t presentModeCount = 0;
        vkGetPhysicalDeviceSurfacePresentModesKHR(m_PhysicalDevice, m_Surface, &presentModeCount, nullptr);
        if (presentModeCount > 0)
        {
            details.presentModes.resize(presentModeCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(m_PhysicalDevice, m_Surface, &presentModeCount, details.presentModes.data());
        }

        return details;
    }

    void VulkanPhysicalDevice::PickPhysicalDevice(VkInstance instance, VkSurfaceKHR surface)
    {
        uint32_t count = 0;
        vkEnumeratePhysicalDevices(instance, &count, nullptr);
        if (count == 0)
            throw std::runtime_error("failed to find GPUs with Vulkan support");

        std::vector<VkPhysicalDevice> devices(count);
        vkEnumeratePhysicalDevices(instance, &count, devices.data());

        VkPhysicalDevice bestDevice = VK_NULL_HANDLE;
        int32_t bestScore = -1;

        for (auto device : devices)
        {
            int32_t score = RateDevice(device, surface);

            VkPhysicalDeviceProperties props{};
            vkGetPhysicalDeviceProperties(device, &props);

            if (score > bestScore)
            {
                bestScore = score;
                bestDevice = device;
            }
        }

        if (bestScore <= 0 || bestDevice == VK_NULL_HANDLE)
            throw std::runtime_error("failed to find suitable GPU");

        m_PhysicalDevice = bestDevice;
        m_QueueFamilies = FindQueueFamilies(m_PhysicalDevice, surface);

        vkGetPhysicalDeviceProperties(m_PhysicalDevice, &m_Properties);
        vkGetPhysicalDeviceFeatures(m_PhysicalDevice, &m_Features);

        VE_CORE_TRACE("Selected GPU: {}", m_Properties.deviceName);
    }

    int32_t VulkanPhysicalDevice::RateDevice(VkPhysicalDevice device, VkSurfaceKHR surface) const
    {
        if (!CheckExtensionSupport(device))
            return 0;

        auto queueFamilies = FindQueueFamilies(device, surface);
        if (!queueFamilies.IsComplete())
            return 0;

        SwapchainSupportDetails swapchain{};
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &swapchain.capabilities);

        uint32_t formatCount = 0;
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);
        if (formatCount == 0)
            return 0;

        uint32_t presentModeCount = 0;
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);
        if (presentModeCount == 0)
            return 0;

        VkPhysicalDeviceProperties props{};
        vkGetPhysicalDeviceProperties(device, &props);

        int32_t score = 0;

        switch (props.deviceType)
        {
        case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
            score += 1000;
        case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
            score += 100;
        case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
            score += 50;
        }

        // VRAM size (mb)
        VkPhysicalDeviceMemoryProperties memProps{};
        vkGetPhysicalDeviceMemoryProperties(device, &memProps);
        for (uint32_t i = 0; i < memProps.memoryHeapCount; i++)
        {
            if (memProps.memoryHeaps[i].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT)
                score += static_cast<int32_t>(memProps.memoryHeaps[i].size / (1024 * 1024));
        }

        if (queueFamilies.transferFamily.has_value())
            score += 50;

        return score;
    }

    bool VulkanPhysicalDevice::CheckExtensionSupport(VkPhysicalDevice device) const
    {
        uint32_t count = 0;
        vkEnumerateDeviceExtensionProperties(device, nullptr, &count, nullptr);
        std::vector<VkExtensionProperties> available(count);
        vkEnumerateDeviceExtensionProperties(device, nullptr, &count, available.data());

        std::set<std::string> required(std::begin(k_RequiredExtensions), std::end(k_RequiredExtensions));

        for (const auto &ext : available)
            required.erase(ext.extensionName);

        return required.empty();
    }

    QueueFamilyIndices VulkanPhysicalDevice::FindQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface) const
    {
        QueueFamilyIndices indices;

        uint32_t count = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &count, nullptr);
        std::vector<VkQueueFamilyProperties> families(count);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &count, families.data());

        for (uint32_t i = 0; i < count; i++)
        {
            const auto &family = families[i];

            // Graphics
            if (family.queueFlags & VK_QUEUE_GRAPHICS_BIT)
                indices.graphicsFamily = i;

            // Present
            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
            if (presentSupport)
                indices.presentFamily = i;

            // Compute
            if (family.queueFlags & VK_QUEUE_COMPUTE_BIT)
                indices.computeFamily = i;

            // Dedicated transfer
            if ((family.queueFlags & VK_QUEUE_TRANSFER_BIT) &&
                !(family.queueFlags & VK_QUEUE_GRAPHICS_BIT) &&
                !(family.queueFlags & VK_QUEUE_COMPUTE_BIT))
            {
                indices.transferFamily = i;
            }
        }

        return indices;
    }

} // namespace ve
