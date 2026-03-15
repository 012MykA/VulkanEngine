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
        VkPipelineVertexInputStateCreateInfo vertexInputInfo{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
            .vertexBindingDescriptionCount = static_cast<uint32_t>(configInfo.bindingDescriptions.size()),
            .pVertexBindingDescriptions = configInfo.bindingDescriptions.data(),
            .vertexAttributeDescriptionCount = static_cast<uint32_t>(configInfo.attributeDescriptions.size()),
            .pVertexAttributeDescriptions = configInfo.attributeDescriptions.data(),
        };

        auto colorBlending = configInfo.colorBlending;
        colorBlending.pAttachments = &configInfo.colorBlendAttachment;

        auto dynamicState = configInfo.dynamicState;
        dynamicState.dynamicStateCount = static_cast<uint32_t>(configInfo.dynamicStateEnables.size());
        dynamicState.pDynamicStates = configInfo.dynamicStateEnables.data();

        // --- Pipeline creation ---
        VkGraphicsPipelineCreateInfo pipelineInfo{
            .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,

            .stageCount = 2,
            .pStages = shaderStages,

            .pVertexInputState = &vertexInputInfo,
            .pInputAssemblyState = &configInfo.inputAssembly,
            .pViewportState = &configInfo.viewportState,
            .pRasterizationState = &configInfo.rasterizer,
            .pMultisampleState = &configInfo.multisampling,
            .pDepthStencilState = &configInfo.depthStencil,
            .pColorBlendState = &colorBlending,
            .pDynamicState = &dynamicState,

            .layout = configInfo.pipelineLayout,
            .renderPass = configInfo.renderPass,
            .subpass = configInfo.subpass,

            .basePipelineHandle = VK_NULL_HANDLE, // Optional
            .basePipelineIndex = -1,              // Optional
        };

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

    PipelineConfig VulkanPipeline::DefaultPipelineConfig()
    {
        PipelineConfig defaultConfig{
            // Input assembly
            .inputAssembly{
                .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
                .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
                .primitiveRestartEnable = VK_FALSE,
            },
            // Viewport & scissor
            .viewportState{
                .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
                .viewportCount = 1,
                .pViewports = nullptr, // Dynamic state
                .scissorCount = 1,
                .pScissors = nullptr, // Dynamic state
            },
            // Rasterizer
            .rasterizer{
                .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
                .depthClampEnable = VK_FALSE,
                .rasterizerDiscardEnable = VK_FALSE,
                .polygonMode = VK_POLYGON_MODE_FILL,
                .cullMode = VK_CULL_MODE_BACK_BIT,
                .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
                .depthBiasEnable = VK_FALSE,
                .depthBiasConstantFactor = 0.0f, // Optional
                .depthBiasClamp = 0.0f,          // Optional
                .depthBiasSlopeFactor = 0.0f,    // Optional
                .lineWidth = 1.0f,
            },
            // Multisampling
            .multisampling{
                .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
                .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
                .sampleShadingEnable = VK_FALSE,
                .minSampleShading = 1.0f,          // Optional
                .pSampleMask = nullptr,            // Optional
                .alphaToCoverageEnable = VK_FALSE, // Optional
                .alphaToOneEnable = VK_FALSE,      // Optional
            },
            // Depth stencil
            .depthStencil{
                .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
                .depthTestEnable = VK_FALSE,  // Switch when depth test enabled
                .depthWriteEnable = VK_FALSE, // Switch when depth test enabled
                .depthCompareOp = VK_COMPARE_OP_LESS,
                .depthBoundsTestEnable = VK_FALSE,
                .stencilTestEnable = VK_FALSE,
                .minDepthBounds = 0.0f, // Optional
                .maxDepthBounds = 1.0f, // Optional
            },
            // Color blending
            .colorBlendAttachment{
                .blendEnable = VK_FALSE,
                .srcColorBlendFactor = VK_BLEND_FACTOR_ONE,  // Optional
                .dstColorBlendFactor = VK_BLEND_FACTOR_ZERO, // Optional
                .colorBlendOp = VK_BLEND_OP_ADD,             // Optional
                .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,  // Optional
                .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO, // Optional
                .alphaBlendOp = VK_BLEND_OP_ADD,             // Optional
                .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                                  VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
            },
            .colorBlending{
                .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
                .logicOpEnable = VK_FALSE,
                .logicOp = VK_LOGIC_OP_COPY, // Optional
                .attachmentCount = 1,
                // .pAttachments = // Written in constructor
                .blendConstants = {0.0f, 0.0f, 0.0f, 0.0f},
            },
            .dynamicStateEnables = {
                VK_DYNAMIC_STATE_VIEWPORT,
                VK_DYNAMIC_STATE_SCISSOR,
            },
            .dynamicState{
                .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
                // .dynamicStateCount = // Written in constructor
                // .pDynamicStates =    // Written in constructor
            },
        };

        return defaultConfig;
    }

} // namespace ve
