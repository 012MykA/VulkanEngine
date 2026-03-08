#include "VulkanPhysicalDevice.hpp"
#include "VulkanEngine/Core/Log.hpp"
#include "Debug/VulkanLogUtils.hpp"

#include <stdexcept>
#include <set>
#include <string>

namespace ve
{
    VulkanPhysicalDevice::VulkanPhysicalDevice(VkPhysicalDevice device, VkSurfaceKHR surface)
        : m_Device(device)
    {
        vkGetPhysicalDeviceProperties(device, &m_Properties);
        vkGetPhysicalDeviceFeatures(device, &m_Features);
        vkGetPhysicalDeviceMemoryProperties(device, &m_MemoryProperties);
        m_QueueIndices = FindQueueIndices(device, surface);
    }

    VulkanPhysicalDevice VulkanPhysicalDevice::Select(VkInstance instance, VkSurfaceKHR surface, const PhysicalDeviceRequirements &req)
    {
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

        if (deviceCount == 0)
            throw std::runtime_error("failed to find GPUs with Vulkan support!");

        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

        VE_CORE_TRACE("Found {0} physical device(s) with Vulkan support.", deviceCount);

        VkPhysicalDevice selectedHandle = VK_NULL_HANDLE;

        for (const auto &handle : devices)
        {
            VkPhysicalDeviceProperties props;
            vkGetPhysicalDeviceProperties(handle, &props);

            VE_CORE_TRACE("Evaluating GPU: {0}", props.deviceName);

            if (IsDeviceSuitable(handle, surface, req))
            {
                // If PreferredDeviceType exists => checking
                if (req.PreferredDeviceType.has_value() && props.deviceType == req.PreferredDeviceType.value())
                {
                    selectedHandle = handle;
                    break;
                }

                if (selectedHandle == VK_NULL_HANDLE)
                    selectedHandle = handle;
            }
        }

        if (selectedHandle == VK_NULL_HANDLE)
            throw std::runtime_error("failed to find a suitable GPU!");

        VulkanPhysicalDevice finalDevice(selectedHandle, surface);
        VE_CORE_INFO("Selected physical device: {0}", finalDevice.GetProperties().deviceName);

        return finalDevice;
    }

    bool VulkanPhysicalDevice::IsDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR surface, const PhysicalDeviceRequirements &req)
    {
        VkPhysicalDeviceProperties properties;
        vkGetPhysicalDeviceProperties(device, &properties);

        VkPhysicalDeviceFeatures features;
        vkGetPhysicalDeviceFeatures(device, &features);

        // Queues
        auto indices = FindQueueIndices(device, surface);
        if (req.RequiresGraphicsQueue && !indices.GraphicsFamily.has_value())
        {
            VE_CORE_WARN("\tDevice rejected: missing graphics queue");
            return false;
        }
        if (req.RequiresPresentQueue && !indices.PresentFamily.has_value())
        {
            VE_CORE_WARN("\tDevice rejected: missing present queue");
            return false;
        }

        // Extensions
        if (!CheckExtensionSupport(device, req.Extensions))
        {
            VE_CORE_WARN("\tDevice rejected: missing required extensions");
            return false;
        }

        // Swapchain
        if (req.SwapchainAdequate)
        {
            auto swapchainSupport = QuerySwapchainSupport(device, surface);
            if (swapchainSupport.Formats.empty() || swapchainSupport.PresentModes.empty())
            {
                VE_CORE_WARN("\tDevice rejected: swapchain inadequate");
                return false;
            }
        }

        // Features
        if (!CheckFeatureSupport(features, req.Features))
        {
            VE_CORE_WARN("\tDevice rejected: missing required features");
            return false;
        }

        return true;
    }

    PhysicalDeviceQueueFamilyIndices VulkanPhysicalDevice::FindQueueIndices(VkPhysicalDevice device, VkSurfaceKHR surface)
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
            if (surface != VK_NULL_HANDLE)
                vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);

            if (presentSupport)
                indices.PresentFamily = i;

            if (indices.IsComplete())
                break;
        }
        return indices;
    }

    bool VulkanPhysicalDevice::CheckExtensionSupport(VkPhysicalDevice device, const std::vector<const char *> &requiredExtensions)
    {
        uint32_t count;
        vkEnumerateDeviceExtensionProperties(device, nullptr, &count, nullptr);
        std::vector<VkExtensionProperties> available(count);
        vkEnumerateDeviceExtensionProperties(device, nullptr, &count, available.data());

        std::set<std::string> requiredSet(requiredExtensions.begin(), requiredExtensions.end());
        for (const auto &ext : available)
            requiredSet.erase(ext.extensionName);

        return requiredSet.empty();
    }

    bool VulkanPhysicalDevice::CheckFeatureSupport(const VkPhysicalDeviceFeatures &supported, const VkPhysicalDeviceFeatures &required)
    {
        auto sPtr = reinterpret_cast<const VkBool32 *>(&supported);
        auto rPtr = reinterpret_cast<const VkBool32 *>(&required);

        for (size_t i = 0; i < sizeof(VkPhysicalDeviceFeatures) / sizeof(VkBool32); i++)
        {
            if (rPtr[i] && !sPtr[i])
                return false;
        }
        return true;
    }

    SwapchainSupportDetails VulkanPhysicalDevice::QuerySwapchainSupport(VkPhysicalDevice device, VkSurfaceKHR surface)
    {
        SwapchainSupportDetails details;
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.Capabilities);
        uint32_t fCount, pCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &fCount, nullptr);
        if (fCount != 0)
        {
            details.Formats.resize(fCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &fCount, details.Formats.data());
        }
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &pCount, nullptr);
        if (pCount != 0)
        {
            details.PresentModes.resize(pCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &pCount, details.PresentModes.data());
        }
        return details;
    }

    SwapchainSupportDetails VulkanPhysicalDevice::GetSwapchainSupport(VkSurfaceKHR surface) const
    {
        return QuerySwapchainSupport(m_Device, surface);
    }

} // namespace ve
