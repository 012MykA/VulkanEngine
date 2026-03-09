#pragma once

#include <vulkan/vulkan.h>

#include <string>

namespace ve
{
    class VulkanPipelineLayout
    {
    public:
        VulkanPipelineLayout(VkDevice device, const std::string &debugName = "Unnamed");
        ~VulkanPipelineLayout();

        VkPipelineLayout GetPipelineLayout() const { return m_PipelineLayout; }

    private:
        VkDevice m_Device;
        VkPipelineLayout m_PipelineLayout;

        std::string m_DebugName;
    };

} // namespace ve
