#pragma once

#include <vulkan/vulkan.h>

#include <vector>
#include <filesystem>
#include <string>

namespace ve
{
    struct PipelineConfigInfo
    {
        std::vector<VkVertexInputBindingDescription> bindingDescriptions;
        std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
        VkPipelineInputAssemblyStateCreateInfo inputAssembly;
        VkPipelineViewportStateCreateInfo viewportState;
        VkPipelineRasterizationStateCreateInfo rasterizer;
        VkPipelineMultisampleStateCreateInfo multisampling;
        VkPipelineDepthStencilStateCreateInfo depthStencil;
        VkPipelineColorBlendAttachmentState colorBlendAttachment;
        VkPipelineColorBlendStateCreateInfo colorBlending;
        std::vector<VkDynamicState> dynamicStateEnables;
        VkPipelineDynamicStateCreateInfo dynamicState;
        VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
        VkRenderPass renderPass = VK_NULL_HANDLE;
        uint32_t subpass = 0;
    };

    class VulkanPipeline
    {
    public:
        VulkanPipeline(VkDevice device,
                       const std::filesystem::path &vertPath,
                       const std::filesystem::path &fragPath,
                       const PipelineConfigInfo &configInfo,
                       const std::string &debugName = "Unnamed pipeline");
        ~VulkanPipeline();

        VulkanPipeline(const VulkanPipeline &) = delete;
        void operator=(const VulkanPipeline &) = delete;

    public:
        void Bind(VkCommandBuffer commandBuffer);

        static void DefaultPipelineConfigInfo(PipelineConfigInfo& config);

    private:
        VkDevice m_Device;
        VkPipeline m_Pipeline = VK_NULL_HANDLE;

        std::string m_DebugName;
    };

} // namespace ve
