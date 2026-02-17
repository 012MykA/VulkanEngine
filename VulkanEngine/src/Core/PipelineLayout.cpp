#include "PipelineLayout.hpp"
#include "Device.hpp"
#include "Validation.hpp"

namespace VE
{
    PipelineLayout::PipelineLayout(const Device &device) : m_Device(device)
    {
        VkPipelineLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        layoutInfo.setLayoutCount = 0;            // Optional
        layoutInfo.pSetLayouts = nullptr;         // Optional
        layoutInfo.pushConstantRangeCount = 0;    // Optional
        layoutInfo.pPushConstantRanges = nullptr; // Optional

        CheckVk(vkCreatePipelineLayout(m_Device.Handle(), &layoutInfo, nullptr, &m_Layout), "create pipeline layout!");
    }

    PipelineLayout::~PipelineLayout()
    {
        vkDestroyPipelineLayout(m_Device.Handle(), m_Layout, nullptr);
    }

}
