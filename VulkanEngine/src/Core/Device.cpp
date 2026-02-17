#include "Device.hpp"
#include "Instance.hpp"
#include "Surface.hpp"
#include "Validation.hpp"

#include <set>

namespace VE
{
    Device::Device(const Instance &instance, const Surface &surface) : m_Instance(instance), m_Surface(surface)
    {
        PickPhysicalDevice();
        CreateLogicalDevice();
    }

    Device::~Device()
    {
        vkDestroyDevice(m_Device, nullptr);
    }

    void Device::PickPhysicalDevice()
    {
        uint32_t deviceCount = 0;
        CheckVk(vkEnumeratePhysicalDevices(m_Instance.Handle(), &deviceCount, nullptr),
                "enumerate physical devices (count)!");

        if (deviceCount == 0)
            throw std::runtime_error("failed to find GPUs with Vulkan support!");

        std::vector<VkPhysicalDevice> devices(deviceCount);
        CheckVk(vkEnumeratePhysicalDevices(m_Instance.Handle(), &deviceCount, devices.data()),
                "enumerate physical devices (data)!");

        for (const auto &device : devices)
        {
            if (IsDeviceSuitable(device))
            {
                m_PhysicalDevice = device;
                break;
            }
        }

        if (m_PhysicalDevice == VK_NULL_HANDLE)
            throw std::runtime_error("failed to find suitable GPU!");
    }

    void Device::CreateLogicalDevice()
    {
        std::set<uint32_t> uniqueQueueFamilies = {
            m_QueueFamilies.GraphicsFamily.value(),
            m_QueueFamilies.PresentFamily.value()};

        float queuePriority = 1.0f;
        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;

        for (uint32_t queueFamily : uniqueQueueFamilies)
        {
            VkDeviceQueueCreateInfo queueCreateInfo{};
            queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo.queueFamilyIndex = queueFamily;
            queueCreateInfo.queueCount = 1;
            queueCreateInfo.pQueuePriorities = &queuePriority;

            queueCreateInfos.push_back(queueCreateInfo);
        }

        VkPhysicalDeviceFeatures deviceFeatures{};

        VkDeviceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

        createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
        createInfo.pQueueCreateInfos = queueCreateInfos.data();
        createInfo.pEnabledFeatures = &deviceFeatures;

        createInfo.enabledExtensionCount = static_cast<uint32_t>(m_DeviceExtensions.size());
        createInfo.ppEnabledExtensionNames = m_DeviceExtensions.data();

        if (!m_Instance.ValidationLayers().empty())
        {
            createInfo.enabledLayerCount = static_cast<uint32_t>(m_Instance.ValidationLayers().size());
            createInfo.ppEnabledLayerNames = m_Instance.ValidationLayers().data();
        }
        else
        {
            createInfo.enabledLayerCount = 0;
            createInfo.ppEnabledLayerNames = nullptr;
        }

        CheckVk(vkCreateDevice(m_PhysicalDevice, &createInfo, nullptr, &m_Device), "create device!");

        vkGetDeviceQueue(m_Device, m_QueueFamilies.GraphicsFamily.value(), 0, &m_GraphicsQueue);
        vkGetDeviceQueue(m_Device, m_QueueFamilies.PresentFamily.value(), 0, &m_PresentQueue);
    }

    bool Device::IsDeviceSuitable(VkPhysicalDevice device)
    {
        auto indices = FindQueueFamilies(device);

        bool extensionsSupported = CheckDeviceExtensionSupport(device);

        bool swapchainAdequate = false;
        if (extensionsSupported)
        {
            auto swapchainSupport = QuerySwapchainSupport(device);
            swapchainAdequate = !swapchainSupport.Formats.empty() && !swapchainSupport.PresentModes.empty();
        }

        return indices.IsComplete() && extensionsSupported && swapchainAdequate;
    }

    bool Device::CheckDeviceExtensionSupport(VkPhysicalDevice device)
    {
        uint32_t extensionCount;
        CheckVk(vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr),
                "enumerate device extension properties (count)!");

        std::vector<VkExtensionProperties> availableExtensions(extensionCount);
        CheckVk(vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data()),
                "enumerate device extension properties (data)!");

        std::set<std::string> requiredExtensions(m_DeviceExtensions.begin(), m_DeviceExtensions.end());

        for (const auto &ext : availableExtensions)
        {
            requiredExtensions.erase(ext.extensionName);
        }

        if (!requiredExtensions.empty())
        {
            std::string extensions;
            for (auto it = requiredExtensions.begin(); it != requiredExtensions.end(); ++it)
            {
                if (it != requiredExtensions.begin())
                    extensions += ", ";
                extensions += *it;
            }

            throw std::runtime_error("missing required extensions: " + extensions);
        }

        return true;
    }

    QueueFamilyIndices Device::FindQueueFamilies(VkPhysicalDevice device)
    {
        QueueFamilyIndices indices;

        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

        for (uint32_t i = 0; i < queueFamilies.size(); ++i)
        {
            const auto &queueFamily = queueFamilies[i];

            if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
                indices.GraphicsFamily = i;

            VkBool32 presentSupport = false;
            CheckVk(vkGetPhysicalDeviceSurfaceSupportKHR(device, i, m_Surface.Handle(), &presentSupport),
                    "get physical device surface support!");

            if (presentSupport)
                indices.PresentFamily = i;

            if (indices.IsComplete())
                break;
        }

        if (indices.IsComplete())
            m_QueueFamilies = indices;

        return indices;
    }

    SwapchainSupportDetails Device::QuerySwapchainSupport(VkPhysicalDevice device) const
    {
        SwapchainSupportDetails details;
        CheckVk(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, m_Surface.Handle(), &details.Capabilities),
                "get physical device surface capabilities KHR!");

        uint32_t formatCount;
        CheckVk(vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_Surface.Handle(), &formatCount, nullptr),
                "get physical device surface formats KHR (count)!");

        if (formatCount != 0)
        {
            details.Formats.resize(formatCount);
            CheckVk(vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_Surface.Handle(), &formatCount, details.Formats.data()),
                    "get physical device surface formats KHR (data)!");
        }

        uint32_t presentModeCount;
        CheckVk(vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_Surface.Handle(), &presentModeCount, nullptr),
                "get physical device surface present modes KHR (count)!");

        if (presentModeCount != 0)
        {
            details.PresentModes.resize(presentModeCount);
            CheckVk(vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_Surface.Handle(), &presentModeCount, details.PresentModes.data()),
                    "get physical device surface present modes KHR (data)!");
        }

        return details;
    }
}
