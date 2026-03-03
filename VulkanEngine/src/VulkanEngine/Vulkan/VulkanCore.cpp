#include "VulkanCore.hpp"
#include "VulkanEngine/Core/Base.hpp"
#include "VulkanEngine/Core/Log.hpp"
#include "Validation.hpp"

namespace ve
{
    VulkanCore::VulkanCore()
    {
    }

    VulkanCore::~VulkanCore()
    {
        vkDestroyInstance(m_Instance, nullptr);
        VE_CORE_TRACE("VkInstance destroyed");
    }

    void VulkanCore::Init(const std::string &appName)
    {
        CreateInstance(appName);
    }

    void VulkanCore::CreateInstance(const std::string &appName)
    {
        const std::vector<const char *> validationLayers = {
            "VK_LAYER_KHRONOS_validation",
        };

        const std::vector<const char *> extensions = {
            VK_KHR_SURFACE_EXTENSION_NAME,
#if defined(VE_PLATFORM_WINDOWS)
            "VK_KHR_win32_surface",
#elif defined(VE_PLATFORM_LINUX)
            "VK_KHR_xcb_surface",
#elif defined(VE_PLATFORM_APPLE)
            "VK_MVK_macos_surface"
#endif

#if defined(VE_DEBUG)
            VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
            VK_EXT_DEBUG_REPORT_EXTENSION_NAME,
#endif
        };

        VE_CORE_TRACE("deviceExtensions:");
        for (auto extension : extensions)
        {
            VE_CORE_TRACE("\t{0}", extension);
        }

        VkApplicationInfo appInfo{};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = appName.c_str();
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = "VulkanEngine";
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
        VE_CORE_TRACE("VkInstance created");
    }

    void VulkanCore::CreateDebugCallback()
    {

    }

} // namespace ve
