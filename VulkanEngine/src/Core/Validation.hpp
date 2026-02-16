#pragma once

#include "vulkan/vulkan.h"

#include <string>

namespace VE
{
    void CheckVk(VkResult result, const std::string &operation);

}
