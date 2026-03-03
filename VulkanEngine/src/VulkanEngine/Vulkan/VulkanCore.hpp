#pragma once

#include <vulkan/vulkan.h>

#include <string>

namespace ve
{
    class VulkanCore
    {
    public:
        VulkanCore();
        ~VulkanCore();

        void Init(const std::string &appName);

    private:
        void CreateInstance(const std::string &appName);

    private:
        VkInstance m_Instance = VK_NULL_HANDLE;
    };

} // namespace ve
