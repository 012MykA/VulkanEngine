#include "VulkanPipeline.hpp"
#include "VulkanShaderModule.hpp"
#include "Debug/VulkanValidation.hpp"
#include "VulkanEngine/Core/Log.hpp"

#include <cassert>

namespace ve
{
    VulkanPipeline::VulkanPipeline(
        VkDevice device,
        const std::filesystem::path &vertPath,
        const std::filesystem::path &fragPath,
        const PipelineConfig &configInfo,
        const std::string &debugName)
        : m_Device(device), m_DebugName(debugName)
    {
        assert(configInfo.pipelineLayout != VK_NULL_HANDLE && "Pipeline layout is null");
        assert(configInfo.renderPass != VK_NULL_HANDLE && "Render pass is null");

        // Shader modules
        VulkanShaderModule vertModule(m_Device, vertPath, m_DebugName + " Vertex");
        VulkanShaderModule fragModule(m_Device, fragPath, m_DebugName + " Fragment");

        VkPipelineShaderStageCreateInfo shaderStages[] = {
            vertModule.CreateShaderStage(VK_SHADER_STAGE_VERTEX_BIT),
            fragModule.CreateShaderStage(VK_SHADER_STAGE_FRAGMENT_BIT),
        };

        // Vertex input
        VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
        vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

        vertexInputInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(configInfo.bindingDescriptions.size());
        vertexInputInfo.pVertexBindingDescriptions = configInfo.bindingDescriptions.data();

        vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(configInfo.attributeDescriptions.size());
        vertexInputInfo.pVertexAttributeDescriptions = configInfo.attributeDescriptions.data();

        // --- Pipeline creation ---
        VkGraphicsPipelineCreateInfo pipelineInfo{};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;

        pipelineInfo.stageCount = 2;
        pipelineInfo.pStages = shaderStages;

        pipelineInfo.pVertexInputState = &vertexInputInfo;
        pipelineInfo.pInputAssemblyState = &configInfo.inputAssembly;
        pipelineInfo.pViewportState = &configInfo.viewportState;
        pipelineInfo.pRasterizationState = &configInfo.rasterizer;
        pipelineInfo.pMultisampleState = &configInfo.multisampling;
        pipelineInfo.pDepthStencilState = &configInfo.depthStencil;
        pipelineInfo.pColorBlendState = &configInfo.colorBlending;
        pipelineInfo.pDynamicState = &configInfo.dynamicState;

        pipelineInfo.layout = configInfo.pipelineLayout;
        pipelineInfo.renderPass = configInfo.renderPass;
        pipelineInfo.subpass = configInfo.subpass;

        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
        pipelineInfo.basePipelineIndex = -1;              // Optional

        VkResult result = vkCreateGraphicsPipelines(m_Device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_Pipeline);
        CHECK_VK_RESULT(result);

        VE_CORE_TRACE("VulkanPipeline ({0}) created", m_DebugName);
    }

    VulkanPipeline::~VulkanPipeline()
    {
        vkDestroyPipeline(m_Device, m_Pipeline, nullptr);
        VE_CORE_TRACE("VulkanPipeline ({0}) destroyed", m_DebugName);
    }

    void VulkanPipeline::Bind(VkCommandBuffer commandBuffer)
    {
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipeline);
    }

    void VulkanPipeline::DefaultPipelineConfig(PipelineConfig &config)
    {
        // Input assembly
        config.inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        config.inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        config.inputAssembly.primitiveRestartEnable = VK_FALSE;
        config.inputAssembly.pNext = nullptr;

        // Viewport & scissor
        config.viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        config.viewportState.viewportCount = 1;
        config.viewportState.pViewports = nullptr; // Dynamic state
        config.viewportState.scissorCount = 1;
        config.viewportState.pScissors = nullptr; // Dynamic state

        // Rasterizer
        config.rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        config.rasterizer.depthClampEnable = VK_FALSE;
        config.rasterizer.rasterizerDiscardEnable = VK_FALSE;
        config.rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
        config.rasterizer.lineWidth = 1.0f;
        config.rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
        config.rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
        config.rasterizer.depthBiasEnable = VK_FALSE;
        config.rasterizer.depthBiasConstantFactor = 0.0f; // Optional
        config.rasterizer.depthBiasClamp = 0.0f;          // Optional
        config.rasterizer.depthBiasSlopeFactor = 0.0f;    // Optional

        // Multisampling
        config.multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        config.multisampling.sampleShadingEnable = VK_FALSE;
        config.multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        config.multisampling.minSampleShading = 1.0f;          // Optional
        config.multisampling.pSampleMask = nullptr;            // Optional
        config.multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
        config.multisampling.alphaToOneEnable = VK_FALSE;      // Optional

        // Depth
        config.depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        config.depthStencil.depthTestEnable = VK_FALSE;  // Switch when depth test enabled
        config.depthStencil.depthWriteEnable = VK_FALSE; // Switch when depth test enabled
        config.depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
        config.depthStencil.depthBoundsTestEnable = VK_FALSE;
        config.depthStencil.stencilTestEnable = VK_FALSE;
        config.depthStencil.minDepthBounds = 0.0f; // Optional
        config.depthStencil.maxDepthBounds = 1.0f; // Optional

        // Color blending
        config.colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                                                     VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        config.colorBlendAttachment.blendEnable = VK_FALSE;
        config.colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;  // Optional
        config.colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
        config.colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;             // Optional
        config.colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;  // Optional
        config.colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
        config.colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;             // Optional

        config.colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        config.colorBlending.logicOpEnable = VK_FALSE;
        config.colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
        config.colorBlending.attachmentCount = 1;
        config.colorBlending.pAttachments = &config.colorBlendAttachment;
        config.colorBlending.blendConstants[0] = 0.0f; // Optional
        config.colorBlending.blendConstants[1] = 0.0f; // Optional
        config.colorBlending.blendConstants[2] = 0.0f; // Optional
        config.colorBlending.blendConstants[3] = 0.0f; // Optional

        config.dynamicStateEnables = {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_SCISSOR,
        };

        config.dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        config.dynamicState.dynamicStateCount = static_cast<uint32_t>(config.dynamicStateEnables.size());
        config.dynamicState.pDynamicStates = config.dynamicStateEnables.data();
    }

} // namespace ve
