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

    private:
        const Device &m_Device;
        VkPipelineLayout m_Layout;
    };

}
