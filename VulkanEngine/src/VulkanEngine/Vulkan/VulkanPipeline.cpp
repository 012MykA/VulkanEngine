#include "VulkanPipeline.hpp"
#include "VulkanLogicalDevice.hpp"
#include "Debug/VulkanValidation.hpp"

#include <fstream>

namespace ve
{
    VulkanShader::VulkanShader(const VulkanLogicalDevice &device, const std::string &spvPath)
        : VulkanShader(device, LoadSpv(spvPath))
    {
        VE_CORE_TRACE("Shader loaded: {}", spvPath);
    }

    VulkanShader::VulkanShader(const VulkanLogicalDevice &device, std::span<const uint32_t> spvCode)
        : m_Device(device.GetVkHandle())
    {
        VkShaderModuleCreateInfo createInfo{
            .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
            .codeSize = spvCode.size() * sizeof(uint32_t),
            .pCode = spvCode.data(),
        };

        VkResult result = vkCreateShaderModule(m_Device, &createInfo, nullptr, &m_Module);
        CHECK_VK_RESULT(result);
    }

    VulkanShader::~VulkanShader()
    {
        if (m_Module != VK_NULL_HANDLE)
            vkDestroyShaderModule(m_Device, m_Module, nullptr);
    }

    std::vector<uint32_t> VulkanShader::LoadSpv(const std::string &path)
    {
        std::ifstream file(path, std::ios::binary | std::ios::ate);
        if (!file.is_open())
            throw std::runtime_error("Failed to open shader: " + path);

        size_t size = static_cast<size_t>(file.tellg());
        if (size % 4 != 0)
            throw std::runtime_error("SPIR-V size not aligned to 4 bytes: " + path);

        file.seekg(0);
        std::vector<uint32_t> code(size / 4);
        file.read(reinterpret_cast<char *>(code.data()), static_cast<std::streamsize>(size));
        return code;
    }

    // --- Vertex ipnut ---
    VertexInputDesc GetPBRVertexInputDesc()
    {
        VertexInputDesc desc;

        desc.bindings.push_back(VkVertexInputBindingDescription{
            .binding = 0,
            .stride = sizeof(float) * 12,
            .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
        });

        desc.attributes = {
            {.location = 0, .binding = 0, .format = VK_FORMAT_R32G32B32_SFLOAT, .offset = 0},                    // position
            {.location = 1, .binding = 0, .format = VK_FORMAT_R32G32B32_SFLOAT, .offset = sizeof(float) * 3},    // normal
            {.location = 2, .binding = 0, .format = VK_FORMAT_R32G32B32A32_SFLOAT, .offset = sizeof(float) * 6}, // tangent
            {.location = 3, .binding = 0, .format = VK_FORMAT_R32G32_SFLOAT, .offset = sizeof(float) * 10},      // uv
        };

        return desc;
    }

    VertexInputDesc GetPositionOnlyVertexInputDesc()
    {
        VertexInputDesc desc;
        desc.bindings.push_back(VkVertexInputBindingDescription{
            .binding = 0,
            .stride = sizeof(float) * 3,
            .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
        });

        desc.attributes.push_back(VkVertexInputAttributeDescription{
            .location = 0,
            .binding = 0,
            .format = VK_FORMAT_R32G32B32_SFLOAT,
            .offset = 0,
        });
        return desc;
    }

    VertexInputDesc GetFullscreenVertexInputDesc()
    {
        return VertexInputDesc{};
    }

    // --- VulkanPipelineLayout ---
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

    // --- VulkanGraphicsPipeline ---
    VulkanGraphicsPipeline::VulkanGraphicsPipeline(const VulkanLogicalDevice &logicalDevice, const GraphicsPipelineDesc &desc)
        : m_Device(logicalDevice.GetVkHandle())
    {
        // Shader stages
        std::vector<VkPipelineShaderStageCreateInfo> shaderStages;

        if (desc.vertexShader != VK_NULL_HANDLE)
        {
            shaderStages.push_back(VkPipelineShaderStageCreateInfo{
                .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                .stage = VK_SHADER_STAGE_VERTEX_BIT,
                .module = desc.vertexShader,
                .pName = desc.vertexEntry,
            });
        }
        if (desc.fragmentShader != VK_NULL_HANDLE)
        {
            shaderStages.push_back({
                .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
                .module = desc.fragmentShader,
                .pName = desc.fragmentEntry,
            });
        }

        // Vertex input
        VkPipelineVertexInputStateCreateInfo vertexInput{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
            .vertexBindingDescriptionCount = static_cast<uint32_t>(desc.vertexInput.bindings.size()),
            .pVertexBindingDescriptions = desc.vertexInput.bindings.empty()
                                              ? nullptr
                                              : desc.vertexInput.bindings.data(),
            .vertexAttributeDescriptionCount = static_cast<uint32_t>(desc.vertexInput.attributes.size()),
            .pVertexAttributeDescriptions = desc.vertexInput.attributes.empty()
                                                ? nullptr
                                                : desc.vertexInput.attributes.data(),
        };

        // Input assembly
        VkPipelineInputAssemblyStateCreateInfo inputAssembly{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
            .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
            .primitiveRestartEnable = VK_FALSE,
        };

        // Dynamic viewport & scissor
        VkPipelineViewportStateCreateInfo viewportState{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
            .viewportCount = 1,
            .pViewports = nullptr, // Dynamic state
            .scissorCount = 1,
            .pScissors = nullptr, // Dynamic state
        };

        // Rasterization
        VkPipelineRasterizationStateCreateInfo rasterization{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
            .depthClampEnable = VK_FALSE,
            .rasterizerDiscardEnable = VK_FALSE,
            .polygonMode = desc.polygonMode,
            .cullMode = desc.cullMode,
            .frontFace = desc.frontFace,
            .depthBiasEnable = VK_FALSE,
            .depthBiasConstantFactor = 0.0f, // Optional
            .depthBiasClamp = 0.0f,          // Optional
            .depthBiasSlopeFactor = 0.0f,    // Optional
            .lineWidth = desc.lineWidth,
        };

        // Multisampling
        VkPipelineMultisampleStateCreateInfo multisampling{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
            .rasterizationSamples = desc.samples,
            .sampleShadingEnable = VK_FALSE,
            .minSampleShading = 1.0f,          // Optional
            .pSampleMask = nullptr,            // Optional
            .alphaToCoverageEnable = VK_FALSE, // Optional
            .alphaToOneEnable = VK_FALSE,      // Optional
        };

        // Depth stencil
        VkPipelineDepthStencilStateCreateInfo depthStencil{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
            .depthTestEnable = desc.depthTest ? VK_TRUE : VK_FALSE,
            .depthWriteEnable = desc.depthWrite ? VK_TRUE : VK_FALSE,
            .depthCompareOp = desc.depthCompareOp,
            .depthBoundsTestEnable = VK_FALSE,
            .stencilTestEnable = VK_FALSE,
            .minDepthBounds = 0.0f, // Optional
            .maxDepthBounds = 1.0f, // Optional
        };

        VkPipelineColorBlendStateCreateInfo blending{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
            .logicOpEnable = VK_FALSE,
            .attachmentCount = 1,
            .pAttachments = &desc.colorBlendAttachment,
        };

        // Dynamic states
        std::vector<VkDynamicState> dynamicStates = {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_SCISSOR,
        };

        VkPipelineDynamicStateCreateInfo dynamicState{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
            .dynamicStateCount = static_cast<uint32_t>(dynamicStates.size()),
            .pDynamicStates = dynamicStates.data(),
        };

        // --- Pipeline ---
        VkGraphicsPipelineCreateInfo pipelineInfo{
            .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
            .stageCount = static_cast<uint32_t>(shaderStages.size()),
            .pStages = shaderStages.data(),
            .pVertexInputState = &vertexInput,
            .pInputAssemblyState = &inputAssembly,
            .pViewportState = &viewportState,
            .pRasterizationState = &rasterization,
            .pMultisampleState = &multisampling,
            .pDepthStencilState = &depthStencil,
            .pColorBlendState = &blending,
            .pDynamicState = &dynamicState,
            .layout = desc.layout,
            .renderPass = desc.renderPass,
            .subpass = 0,
        };

        VkResult result = vkCreateGraphicsPipelines(m_Device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_Pipeline);
        CHECK_VK_RESULT(result);

        VE_CORE_TRACE("GraphicsPipeline created");
    }

    VulkanGraphicsPipeline::~VulkanGraphicsPipeline()
    {
        if (m_Pipeline != VK_NULL_HANDLE)
            vkDestroyPipeline(m_Device, m_Pipeline, nullptr);
    }

    void VulkanGraphicsPipeline::Bind(VkCommandBuffer cmd) const
    {
        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipeline);
    }

    // -- Blendings ---

    // Opaque blending
    VkPipelineColorBlendAttachmentState MakeOpaqueBlend()
    {
        return VkPipelineColorBlendAttachmentState{
            .blendEnable = VK_FALSE,
            .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                              VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
        };
    }

    // Additive blending
    VkPipelineColorBlendAttachmentState MakeAdditiveBlend()
    {
        return VkPipelineColorBlendAttachmentState{
            .blendEnable = VK_TRUE,
            .srcColorBlendFactor = VK_BLEND_FACTOR_ONE,
            .dstColorBlendFactor = VK_BLEND_FACTOR_ONE,
            .colorBlendOp = VK_BLEND_OP_ADD,
            .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
            .dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
            .alphaBlendOp = VK_BLEND_OP_ADD,
            .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                              VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
        };
    }

    // Alpha blending
    VkPipelineColorBlendAttachmentState MakeAlphaBlend()
    {
        return VkPipelineColorBlendAttachmentState{
            .blendEnable = VK_TRUE,
            .srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
            .dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
            .colorBlendOp = VK_BLEND_OP_ADD,
            .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
            .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
            .alphaBlendOp = VK_BLEND_OP_ADD,
            .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                              VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
        };
    }

} // namespace ve
