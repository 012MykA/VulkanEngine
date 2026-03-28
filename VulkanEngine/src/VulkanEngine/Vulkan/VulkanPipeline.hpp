#pragma once

#include <vulkan/vulkan.h>

#include <vector>
#include <string>
#include <span>
#include <cstdint>

namespace ve
{
    class VulkanLogicalDevice;

    // --- VulkanShader (SPIR-V module) ---
    class VulkanShader
    {
    public:
        VulkanShader(VkDevice device, const std::string &spvPath);
        VulkanShader(VkDevice device, std::span<const uint32_t> spvCode);
        ~VulkanShader();

        VulkanShader(const VulkanShader &) = delete;
        VulkanShader &operator=(const VulkanShader &) = delete;

        VkShaderModule GetVkHandle() const { return m_Module; }

    private:
        static std::vector<uint32_t> LoadSpv(const std::string &path);

    private:
        VkDevice m_Device = VK_NULL_HANDLE;
        VkShaderModule m_Module = VK_NULL_HANDLE;
    };

    // --- Vertex ipnut ---
    struct VertexInputDesc
    {
        std::vector<VkVertexInputBindingDescription> bindings;
        std::vector<VkVertexInputAttributeDescription> attributes;
    };

    VertexInputDesc GetPBRVertexInputDesc();

    VertexInputDesc GetPositionOnlyVertexInputDesc();

    VertexInputDesc GetFullscreenVertexInputDesc();

    // --- GraphicsPipelineDesc ---
    struct GraphicsPipelineDesc
    {
        // Shaders
        VkShaderModule vertexShader = VK_NULL_HANDLE;
        const char *vertexEntry = "main";
        VkShaderModule fragmentShader = VK_NULL_HANDLE;
        const char *fragmentEntry = "main";

        // Vertices
        VertexInputDesc vertexInput;

        // --- dynamic Viewport/Scissor
        bool dynamicViewport = true;

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

        // Blending
        std::vector<VkPipelineColorBlendAttachmentState> blendAttachments;

        // Dynamic Rendering (Vulkan 1.3, without RenderPass)
        std::vector<VkFormat> colorAttachmentFormats;
        VkFormat depthAttachmentFormat = VK_FORMAT_UNDEFINED;
        VkFormat stencilAttachmentFormat = VK_FORMAT_UNDEFINED;

        // Layout
        VkPipelineLayout layout = VK_NULL_HANDLE;
    };

    // --- VulkanPipelineLayout ---
    class VulkanPipelineLayout
    {
    public:
        class Builder
        {
        public:
            explicit Builder(const VulkanLogicalDevice &logicalDevice);

            Builder &AddDescriptorSetLayout(VkDescriptorSetLayout layout);

            template <typename T>
            Builder &AddPushConstantRange(VkShaderStageFlags stages, uint32_t offset = 0)
            {
                m_PushConstants.push_back(VkPushConstantRange{
                    .stageFlags = stages,
                    .offset = offset,
                    .size = sizeof(T),
                });
                return *this;
            }

            [[nodiscard]] VulkanPipelineLayout Build() const;

        private:
            VkDevice m_Device = VK_NULL_HANDLE;
            std::vector<VkDescriptorSetLayout> m_SetLayouts;
            std::vector<VkPushConstantRange> m_PushConstants;
        };

    public:
        VulkanPipelineLayout(VkDevice device, VkPipelineLayout layout);
        ~VulkanPipelineLayout();

        VulkanPipelineLayout(const VulkanPipelineLayout &) = delete;
        VulkanPipelineLayout &operator=(const VulkanPipelineLayout &) = delete;

        VulkanPipelineLayout(VulkanPipelineLayout &&other) noexcept;
        VulkanPipelineLayout &operator=(VulkanPipelineLayout &&other) noexcept;

        VkPipelineLayout GetVkHandle() const { return m_Layout; }

    private:
        VkDevice m_Device = VK_NULL_HANDLE;
        VkPipelineLayout m_Layout = VK_NULL_HANDLE;
    };

    // --- VulkanGraphicsPipeline ---
    //
    // Using Vulkan 1.3 for dynamic rendering (VK_KHR_dynamic_rendering)
    // ---
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
