#pragma once

#include <vulkan/vulkan.h>

namespace VE
{
    class Device;

    class PipelineLayout final
    {
    public:
        PipelineLayout(const Device &device);
        ~PipelineLayout();

        VkPipelineLayout Handle() const { return m_Layout; }

    private:
        const Device &m_Device;
        VkPipelineLayout m_Layout = VK_NULL_HANDLE;
    };

}
