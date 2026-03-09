#include "VulkanPipelineLayout.hpp"
#include "Debug/VulkanValidation.hpp"
#include "VulkanEngine/Core/Log.hpp"

namespace ve
{
    VulkanPipelineLayout::VulkanPipelineLayout(VkDevice device, const std::string &debugName)
        : m_Device(device), m_DebugName(debugName)
    {
        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 0;            // Optional
        pipelineLayoutInfo.pSetLayouts = nullptr;         // Optional
        pipelineLayoutInfo.pushConstantRangeCount = 0;    // Optional
        pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional

        VkResult result = vkCreatePipelineLayout(m_Device, &pipelineLayoutInfo, nullptr, &m_PipelineLayout);
        CHECK_VK_RESULT(result);
        VE_CORE_TRACE("VulkanPipelineLayout ({0}) created", m_DebugName);
    }

    VulkanPipelineLayout::~VulkanPipelineLayout()
    {
        vkDestroyPipelineLayout(m_Device, m_PipelineLayout, nullptr);
        VE_CORE_TRACE("VulkanPipelineLayout ({0}) destroyed", m_DebugName);
    }

} // namespace ve
