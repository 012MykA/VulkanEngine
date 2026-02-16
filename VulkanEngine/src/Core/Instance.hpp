#pragma once

#include <vulkan/vulkan.h>

#include <string>

namespace VE
{
    class Window;

    class Instance
    {
    public:
        Instance(const std::string &appName, const Window &window);
        ~Instance();

    private:
        VkInstance m_Instance = VK_NULL_HANDLE;
    };

}
