#pragma once

#include <vulkan/vulkan.h>

#include <string>
#include <vector>

namespace ve::validation
{
    void CheckVk(VkResult result, const std::string &operation);

    void CheckValidationLayerSupport(const std::vector<const char *> &validationLayers);

} // namespace ve::validation
