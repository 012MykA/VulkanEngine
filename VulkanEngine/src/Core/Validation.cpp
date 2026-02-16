#include "Validation.hpp"

#include "vulkan/vk_enum_string_helper.h"

#include <stdexcept>
#include <iostream>
#include <format>

namespace VE
{
    void VE::CheckVk(VkResult result, const std::string &message)
    {
        if (result == VK_SUCCESS)
            throw std::runtime_error(std::format("{} ({})", message, string_VkResult(result)));
    }

}
