#include "VulkanPipelineLayout.hpp"
#include "VulkanLogicalDevice.hpp"

namespace ve
{
    VulkanPipelineLayout::Builder::Builder(const VulkanLogicalDevice &logicalDevice)
        : m_Device(logicalDevice.GetVkHandle())
    {
    }

    VulkanPipelineLayout::Builder &VulkanPipelineLayout::Builder::AddDescriptorSetLayout(VkDescriptorSetLayout layout)
    {
        m_SetLayouts.push_back(layout);
        return *this;
    }

    VulkanPipelineLayout VulkanPipelineLayout::Builder::Build() const
    {
        VkPipelineLayoutCreateInfo createInfo{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
            .setLayoutCount = static_cast<uint32_t>(m_SetLayouts.size()),
            .pSetLayouts = m_SetLayouts.empty() ? nullptr : m_SetLayouts.data(),
            .pushConstantRangeCount = static_cast<uint32_t>(m_PushConstants.size()),
            .pPushConstantRanges = m_PushConstants.empty() ? nullptr : m_PushConstants.data(),
        };

        VkPipelineLayout layout = VK_NULL_HANDLE;
        VkResult result = vkCreatePipelineLayout(m_Device, &createInfo, nullptr, &layout);
        CHECK_VK_RESULT(result);

        return VulkanPipelineLayout(m_Device, layout);
    }

    VulkanPipelineLayout::VulkanPipelineLayout(VkDevice device, VkPipelineLayout layout)
        : m_Device(device), m_Layout(layout)
    {
    }

    VulkanPipelineLayout::~VulkanPipelineLayout()
    {
        if (m_Layout != VK_NULL_HANDLE)
            vkDestroyPipelineLayout(m_Device, m_Layout, nullptr);
    }

    VulkanPipelineLayout::VulkanPipelineLayout(VulkanPipelineLayout &&other) noexcept
        : m_Device(other.m_Device), m_Layout(other.m_Layout)
    {
        other.m_Layout = VK_NULL_HANDLE;
    }

    VulkanPipelineLayout &VulkanPipelineLayout::operator=(VulkanPipelineLayout &&other) noexcept
    {
        if (this != &other)
        {
            if (m_Layout != VK_NULL_HANDLE)
                vkDestroyPipelineLayout(m_Device, m_Layout, nullptr);
            m_Device = other.m_Device;
            m_Layout = other.m_Layout;
            other.m_Layout = VK_NULL_HANDLE;
        }
        return *this;
    }

} // namespace ve
