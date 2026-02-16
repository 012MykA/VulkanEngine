#include "Instance.hpp"
#include "VulkanEngine/Core/Window.hpp"
#include "Validation.hpp"

#include <stdexcept>
#include <iostream>

namespace VE
{
    Instance::Instance(const std::string &appName, const Window &window)
    {
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

        auto glfwExtensions = window.GetRequiredInstanceExtensions();
        createInfo.enabledExtensionCount = static_cast<uint32_t>(glfwExtensions.size());
        createInfo.ppEnabledExtensionNames = glfwExtensions.data();
        createInfo.enabledLayerCount = 0;
        createInfo.ppEnabledLayerNames = nullptr;

        CheckVk(vkCreateInstance(&createInfo, nullptr, &m_Instance), "failed to create instance!");
    }

    Instance::~Instance()
    {
        if (m_Instance != VK_NULL_HANDLE)
            vkDestroyInstance(m_Instance, nullptr);
        m_Instance = VK_NULL_HANDLE;
    }

}
