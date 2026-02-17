#pragma once

#include <vulkan/vulkan.h>

namespace VE
{
    class Instance;
    class Window;

    class Surface final
    {
    public:
        Surface(const Instance &instance, const Window &window);
        ~Surface();

        VkSurfaceKHR Handle() const { return m_Surface; }

    private:
        const Instance &m_Instance;
        VkSurfaceKHR m_Surface = VK_NULL_HANDLE;
    };

}
