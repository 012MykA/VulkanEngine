#pragma once

#include "VulkanEngine/Core/Base.hpp"

#include <cstdint>
#include <string>

namespace ve::rhi
{
    // --- Handles -------------------------
    struct BufferHandle
    {
        uint64_t id = 0;
        bool IsValid() const { return id != 0; }
    };

    struct TextureHandle
    {
        uint64_t id = 0;
        bool IsValid() const { return id != 0; }
    };

    struct ShaderHandle
    {
        uint64_t id = 0;
        bool IsValid() const { return id != 0; }
    };

    struct PipelineHandle
    {
        uint64_t id = 0;
        bool IsValid() const { return id != 0; }
    };

    struct RenderPassHandle
    {
        uint64_t id = 0;
        bool IsValid() const { return id != 0; }
    };

    struct SamplerHandle
    {
        uint64_t id = 0;
        bool IsValid() const { return id != 0; }
    };

    struct FenceHandle
    {
        uint64_t id = 0;
        bool IsValid() const { return id != 0; }
    };

    // --- Formats -------------------------
    enum class Format : uint32_t
    {
        Undefined,
        R8_UNORM,
        R8G8B8A8_UNORM,
        R8G8B8A8_SRGB,
        B8G8R8A8_UNORM,
        B8G8R8A8_SRGB,
        R16G16B16A16_FLOAT,
        R32G32B32A32_FLOAT,
        R32G32B32_FLOAT,
        R32G32_FLOAT,
        R32_FLOAT,
        D32_FLOAT,
        D24_UNORM_S8_UINT,
        D16_UNORM,
        BC1_RGB_UNORM,
        BC3_UNORM,
        BC7_UNORM
    };

    // --- Buffers -------------------------
    // clang-format off
    enum class BufferUsage : uint32_t
    {
        None        = 0,
        Vertex      = BIT(0),
        Index       = BIT(1),
        Uniform     = BIT(2),
        Storage     = BIT(3),
        TransferSrc = BIT(4),
        TransferDst = BIT(5),
        Indirect    = BIT(6)
    };
    // clang-format on

    inline BufferUsage operator|(BufferUsage a, BufferUsage b)
    {
        return static_cast<BufferUsage>(uint32_t(a) | uint32_t(b));
    }

    inline bool operator&(BufferUsage a, BufferUsage b)
    {
        return (uint32_t(a) & uint32_t(b)) != 0;
    }

    enum class MemoryUsage
    {
        GPU_Only,
        CPU_To_GPU,
        GPU_To_CPU,
        CPU_Only
    };

    // clang-format off
    struct BufferDesc
    {
        uint64_t size           = 0;
        BufferUsage usage       = BufferUsage::None;
        MemoryUsage memoryUsage = MemoryUsage::GPU_Only;
        const char *debugName   = nullptr;
    };
    // clang-format on

    // --- Textures -------------------------
    enum class TextureType
    {
        Texture2D,
        Texture3D,
        TextureCube,
        Texture2DArray
    };

    // clang-format off
    enum class TextureUsage : uint32_t
    {
        None         = 0,
        Sampled      = BIT(0),
        Storage      = BIT(1),
        RenderTarget = BIT(2),
        DepthStencil = BIT(3),
        TransferSrc  = BIT(4),
        TransferDst  = BIT(5),
    };
    // clang-format on

    struct TextureDesc
    {
        uint32_t width = 1;
        uint32_t height = 1;
        uint32_t depth = 1;
        uint32_t mipLevels = 1;
        uint32_t arrayLayers = 1;
        Format format = Format::R8G8B8A8_UNORM;
        TextureType type = TextureType::Texture2D;
        TextureUsage usage = TextureUsage::Sampled;
        const char *debugName = nullptr;
    };

    // Shaders
    // clang-format off
    enum class ShaderStage : uint32_t
    {
        Vertex   = BIT(0),
        Fragment = BIT(1),
        Compute  = BIT(2),
        Geometry = BIT(3)
    };
    // clang-format on

    struct ShaderDesc
    {
        ShaderStage stage;
        const uint32_t *spirvCode = nullptr; // SPIR-V for Vulkan
        size_t spirvSize = 0;
        const char *glslSource = nullptr; // GLSL for OpenGL
        const char *hlslSource = nullptr; // HLSL for D3D12
        const char *entryPoint = "main";
        const char *debugName = nullptr;
    };

    // --- Pipelines -------------------------
    enum class PrimitiveTopology
    {
        TriangleList,
        TriangleStrip,
        LineList,
        PointList,
    };

    enum class PolygonMode
    {
        Fill,
        Line,
        Point,
    };

    enum class CullMode
    {
        None,
        Front,
        Back
    };

    enum class FrontFace
    {
        Clockwise,
        CounterClockwise,
    };

    enum class CompareOp
    {
        Never,
        Less,
        Equal,
        LessOrEqual,
        Greater,
        NotEqual,
        GreaterOrEqual,
        Always
    };

    enum class BlendFactor
    {
        Zero,
        One,
        SrcAlpha,
        OneMinusSrcAlpha,
        DstAlpha,
        OneMinusDstAlpha
    };

    enum class BlendOp
    {
        Add,
        Subtract,
        ReverseSubtract,
        Min,
        Max
    };

    enum class StencilOp
    {
        Keep,
        Zero,
        Replace,
        IncrementClamp,
        DecrementClamp,
        Invert
    };

    struct RasterizationState
    {
        PolygonMode polygonMode = PolygonMode::Fill;
        CullMode cullMode = CullMode::Back;
        FrontFace frontFace = FrontFace::CounterClockwise;
        bool depthClampEnable = false;
        float lineWidth = 1.0f;
    };

    struct DepthStencilState
    {
        bool depthTestEnable = true;
        bool depthWriteEnable = true;
        CompareOp depthCompareOp = CompareOp::Less;
        bool stencilEnable = false;
    };

    struct BlendAttachment
    {
        bool blendEnable = false;
        BlendFactor srcColorFactor = BlendFactor::SrcAlpha;
        BlendFactor dstColorFactor = BlendFactor::OneMinusSrcAlpha;
        BlendOp colorOp = BlendOp::Add;
        BlendFactor srcAlphaFactor = BlendFactor::One;
        BlendFactor dstAlphaFactor = BlendFactor::Zero;
        BlendOp alphaOp = BlendOp::Add;
    };

    struct VertexAttribute
    {
        uint32_t location = 0;
        uint32_t binding = 0;
        Format format = Format::R32G32B32A32_FLOAT;
        uint32_t offset = 0;
    };

    struct VertexBinding
    {
        uint32_t binding = 0;
        uint32_t stride = 0;
        bool perInstance = false;
    };

    struct GraphicsPipelineDesc
    {
        ShaderHandle vertexShader;
        ShaderHandle fragmentShader;
        ShaderHandle geometryShader; // Optional
        RenderPassHandle renderPass;
        uint32_t subpass = 0;

        std::vector<VertexAttribute> vertexAttributes;
        std::vector<VertexBinding> vertexBindings;
        PrimitiveTopology topology = PrimitiveTopology::TriangleList;
        RasterizationState rasterization = {};
        DepthStencilState depthStencil = {};
        std::vector<BlendAttachment> blendAttachments;
        const void *pipelineLayout = nullptr;

        const char *debugName = nullptr;
    };

    struct ComputePipelineDesc
    {
        ShaderHandle computeShader;
        const void *pipelineLayout = nullptr;
        const char *debugName = nullptr;
    };

    // --- RenderPass -------------------------
    enum class LoadOp
    {
        Load,
        Clear,
        DontCare
    };

    enum class StoreOp
    {
        Store,
        DontCare
    };

    struct AttachmentDesc
    {
        Format format = Format::R8G8B8A8_UNORM;
        LoadOp loadOp = LoadOp::Clear;
        StoreOp storeOp = StoreOp::Store;
        LoadOp stencilLoad = LoadOp::DontCare;
        StoreOp stencilStore = StoreOp::DontCare;
        bool isDepth = false;
    };

    struct RenderPassDesc
    {
        std::vector<AttachmentDesc> colorAttachment;
        AttachmentDesc depthAttachment;
        bool hasDepth = false;
        const char *debugName = nullptr;
    };

    // --- Commands -------------------------
    struct Viewport
    {
        float x = 0, y = 0;
        float width, height;
        float minDepth = 0.0f, maxDepth = 1.0f;
    };

    struct Scissor
    {
        int32_t x = 0, y = 0;
        uint32_t width, height;
    };

    struct ClearValue
    {
        float r = 0.0f, g = 0.0f, b = 0.0f, a = 1.0f;
        float depth = 1.0f;
        uint8_t stencil = 0;
    };

} // namespace ve::rhi
