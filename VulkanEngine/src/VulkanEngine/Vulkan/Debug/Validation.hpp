#pragma once

#include "VulkanEngine/Core/Log.hpp"

#include <vulkan/vulkan.h>
#include <vulkan/vk_enum_string_helper.h>

#include <stdexcept>

#define CHECK_VK_RESULT(result)                                                                         \
    do                                                                                                  \
    {                                                                                                   \
        if (result != VK_SUCCESS)                                                                       \
        {                                                                                               \
            VE_CORE_ERROR("Vulkan error: {0} at {1}:{2}", string_VkResult(result), __FILE__, __LINE__); \
            throw std::runtime_error("Vulkan error occurred!");                                         \
        }                                                                                               \
    } while (0)

namespace ve
{
} // namespace ve
