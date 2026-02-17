#include "Instance.hpp"
#include "VulkanEngine/Core/Window.hpp"
#include "Validation.hpp"

#include <stdexcept>
#include <iostream>

namespace VE
{
    Instance::Instance(const std::string &appName, const Window &window, const std::vector<const char *> &validationLayers)
        : m_ValidationLayers(validationLayers)
    {
        CheckVulkanValidationLayerSupport(validationLayers);

        auto extensions = window.GetRequiredInstanceExtensions();

        if (!validationLayers.empty())
            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

        VkApplicationInfo appInfo{};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = appName.c_str();
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = "No Engine";
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_3;

        VkInstanceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;

        createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
        createInfo.ppEnabledExtensionNames = extensions.data();

        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();

        CheckVk(vkCreateInstance(&createInfo, nullptr, &m_Instance), "create instance!");
    }

    Instance::~Instance()
    {
        vkDestroyInstance(m_Instance, nullptr);
    }

    void Instance::CheckVulkanValidationLayerSupport(const std::vector<const char *> &validationLayers)
    {
        uint32_t layerCount;
        CheckVk(vkEnumerateInstanceLayerProperties(&layerCount, nullptr),
                "enumerate instance layer properties (count)!");

        std::vector<VkLayerProperties> availableLayers(layerCount);
        CheckVk(vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data()),
                "enumerate instance layer properties (data)!");

        for (const char *layer : validationLayers)
        {
            auto result = std::find_if(availableLayers.begin(), availableLayers.end(),
                                       [layer](const VkLayerProperties &layerProperties)
                                       {
                                           return strcmp(layer, layerProperties.layerName) == 0;
                                       });

            if (result == availableLayers.end())
            {
                throw std::runtime_error(std::format("could not find the requested validation layer: '{}'!", layer));
            }
        }
    }
}
