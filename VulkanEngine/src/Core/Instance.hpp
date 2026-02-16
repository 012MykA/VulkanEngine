#pragma once

#include <vulkan/vulkan.h>

#include <string>
#include <vector>

namespace VE
{
    class Window;

    class Instance final
    {
    public:
        Instance(const std::string &appName, const Window &window, const std::vector<const char *> &validationLayers);
        ~Instance();

        VkInstance Handle() const { return m_Instance; }

        const std::vector<const char *> &ValidationLayers() const { return m_ValidationLayers; }

    private:
        void CheckVulkanValidationLayerSupport(const std::vector<const char *> &validationLayers);

    private:
        VkInstance m_Instance = VK_NULL_HANDLE;
        const std::vector<const char *> m_ValidationLayers;
    };

}
