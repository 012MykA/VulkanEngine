#include "PipelineLayout.hpp"
#include "Device.hpp"
#include "DescriptorSetLayout.hpp"
#include "Validation.hpp"

namespace VE
{
    PipelineLayout::PipelineLayout(const Device &device, const DescriptorSetLayout &descriptorSetLayout)
        : m_Device(device)
    {
        VkDescriptorSetLayout descriptorSetLayouts[] = {descriptorSetLayout.Handle()};

        VkPipelineLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        layoutInfo.setLayoutCount = 1;
        layoutInfo.pSetLayouts = descriptorSetLayouts;
        layoutInfo.pushConstantRangeCount = 0;    // Optional
        layoutInfo.pPushConstantRanges = nullptr; // Optional

        CheckVk(vkCreatePipelineLayout(m_Device.Handle(), &layoutInfo, nullptr, &m_Layout), "create pipeline layout!");
    }

    PipelineLayout::~PipelineLayout()
    {
        vkDestroyPipelineLayout(m_Device.Handle(), m_Layout, nullptr);
    }

}
