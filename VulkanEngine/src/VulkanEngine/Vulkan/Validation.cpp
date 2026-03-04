#include "Validation.hpp"
#include "VulkanEngine/Core/Base.hpp"

#if defined(VE_DEBUG)
    #include <vulkan/vk_enum_string_helper.h>
    #include <stdexcept>
    #include <format>
#endif

#include <cstring>

namespace ve::validation
{
    void CheckVk(VkResult result, const std::string &operation)
    {
#if defined(VE_DEBUG)
            if (result != VK_SUCCESS)
                throw std::runtime_error(std::format("failed to {} ({})", operation, string_VkResult(result)));
#endif
    }

    void CheckValidationLayerSupport(const std::vector<const char *> &validationLayers)
    {
        uint32_t layerCount;
        CheckVk(vkEnumerateInstanceLayerProperties(&layerCount, nullptr),
                "enumerate instance layer properties (count)");

        std::vector<VkLayerProperties> availableLayers(layerCount);
        CheckVk(vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data()),
                "enumerate instance layer properties (data)");

        for (const char *layer : validationLayers)
        {
            auto result = std::find_if(availableLayers.begin(), availableLayers.end(),
                                       [layer](const VkLayerProperties &layerProperties)
                                       {
                                           return std::strcmp(layer, layerProperties.layerName) == 0;
                                       });

            if (result == availableLayers.end())
            {
                throw std::runtime_error(std::format("could not find the requested validation layer: '{}'!", layer));
            }
        }
    }

} // namespace ve::validation
