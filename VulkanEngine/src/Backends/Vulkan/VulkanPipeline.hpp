#pragma once

#include <vulkan/vulkan.h>

#include <vector>

namespace ve
{
    class VulkanLogicalDevice;

    /**
     * Vulkan Graphics Pipeline
     */
    struct VertexInputDesc
    {
        std::vector<VkVertexInputBindingDescription> bindings;
        std::vector<VkVertexInputAttributeDescription> attributes;
    };

    VertexInputDesc GetPBRVertexInputDesc();

    struct GraphicsPipelineDesc
    {
        // Shaders
        VkShaderModule vertexShader = VK_NULL_HANDLE;
        const char *vertexEntry = "main";
        VkShaderModule fragmentShader = VK_NULL_HANDLE;
        const char *fragmentEntry = "main";

        // Vertices
        VertexInputDesc vertexInput;

        // Rasterization
        VkPolygonMode polygonMode = VK_POLYGON_MODE_FILL;
        VkCullModeFlags cullMode = VK_CULL_MODE_BACK_BIT;
        VkFrontFace frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
        float lineWidth = 1.0f;

        // Multisampling
        VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT;

        // Depth/Stencil
        bool depthTest = true;
        bool depthWrite = true;
        VkCompareOp depthCompareOp = VK_COMPARE_OP_LESS;

        // Color blending
        VkPipelineColorBlendAttachmentState colorBlendAttachment;

        VkRenderPass renderPass = VK_NULL_HANDLE;
        VkPipelineLayout layout = VK_NULL_HANDLE;
    };

    class VulkanGraphicsPipeline
    {
    public:
        explicit VulkanGraphicsPipeline(const VulkanLogicalDevice &logicalDevice,
                                        const GraphicsPipelineDesc &desc);

        ~VulkanGraphicsPipeline();

        VulkanGraphicsPipeline(const VulkanGraphicsPipeline &) = delete;
        VulkanGraphicsPipeline &operator=(const VulkanGraphicsPipeline &) = delete;

        void Bind(VkCommandBuffer cmd) const;

        VkPipeline GetVkHandle() const { return m_Pipeline; }

    private:
        VkDevice m_Device = VK_NULL_HANDLE;
        VkPipeline m_Pipeline = VK_NULL_HANDLE;
    };

    // -- Blendings ---

    // Opaque blending - one attachment
    VkPipelineColorBlendAttachmentState MakeOpaqueBlend();

    // Additive blending - for emissive / lights
    VkPipelineColorBlendAttachmentState MakeAdditiveBlend();

    // Alpha blending - for transparent
    VkPipelineColorBlendAttachmentState MakeAlphaBlend();

} // namespace ve
