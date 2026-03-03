#pragma once

#include <vulkan/vulkan.h>

#include <string>

namespace ve
{
    void CheckVk(VkResult result, const std::string &operation);

} // namespace ve
