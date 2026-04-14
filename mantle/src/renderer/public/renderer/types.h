#pragma once
#include <span>
#include "core/enum_flags.h"
#include <array>
#include <string_view>

namespace mantle {
    struct BufferHandle final {
        u32 index;
        u32 generation;
    };
    struct ImageHandle final {
        u32 index;
        u32 generation;
    };
    struct SamplerHandle final {
        u32 index;
        u32 generation;
    };
    struct ShaderHandle final {
        u32 index;
        u32 generation;
    };
    struct GraphicsPipelineHandle final {
        u32 index;
        u32 generation;
    };
    struct ComputePipelineHandle final {
        u32 index;
        u32 generation;
    };

    struct RGBufferHandle final {
        u32 index;
    };
    struct RGImageHandle final {
        u32 index;
    };

    enum class BufferUsage{
        None = 0,
        Vertex = 1 << 0,
        Index = 1 << 1,
        Uniform = 1 << 2,
        Storage = 1 << 3,
        Transfer = 1 << 4,
        MaxEnum = (1 << 5) - 1,
    };
    enum class MemoryType {
        Gpu,
        CpuToGpu,
    };
    struct BufferDesc final {
        usize size = 0;
        BufferUsage usage = {};
        MemoryType memory = MemoryType::Gpu;
    };

    enum class ImageFormat {
        Undefined,
        Rgba8,
        Rgba8Srgb,
        Bgra8Srgb,
        Rgba16f,
        R32f,
        D16,
        D32,
        D16S8,
        D24S8,
        D32S8,
    };
    enum class ImageUsage{
        None = 0,
        Sampled = 1 << 0,
        Storage = 1 << 1,
        Color = 1 << 2,
        Depth = 1 << 3,
        TransferSrc = 1 << 4,
        TransferDst = 1 << 5,
        MaxEnum = (1 << 6) - 1,
    };
    enum class SampleCount {
        x1 = 1,
        x2 = 2,
        x4 = 4,
        x8 = 8,
    };
    struct ImageDesc final {
        u32 width = 0;
        u32 height = 0;
        u32 depth = 1;
        u32 mip_levels = 1;
        u32 array_layers = 1;
        SampleCount sample_count = SampleCount::x1;
        ImageFormat format = {};
        ImageUsage usage = {};
        bool create_view = true;
    };

    enum class ShaderStage {
        Vertex,
        TessellationControl,
        TessellationEvaluation,
        Fragment,
        Compute,
    };
    struct ShaderModule final {
        std::string_view entry_point = "main";
        ShaderStage stage;
        ShaderHandle shader;
    };

    enum class VertexFormat{
        Float1,
        Float2,
        Float3,
        Float4,
    };
    struct VertexAttribute final {
        u32 location = 0;
        u32 binding = 0;
        VertexFormat format = VertexFormat::Float3;
        u32 offset = 0;
    };
    struct VertexBinding final {
        u32 binding = 0;
        u32 stride = 0;
        bool per_instance = false;
    };
    struct VertexInputState final {
        std::span<const VertexBinding> bindings;
        std::span<const VertexAttribute> attributes;
    };

    enum class PrimitiveTopology {
        PointList,
        LineList,
        LineStrip,
        TriangleList,
        TriangleStrip,
        TriangleFan,
    };
    struct InputAssemblyState final {
        PrimitiveTopology topology = PrimitiveTopology::TriangleList;
        bool primitive_restart_enable = false;
    };

    struct TessellationState final {
        u32 patch_control_points = 3;
    };

    enum class PolygonMode {
        Fill,
        Line,
        Point,
    };
    enum class CullMode {
        None,
        Front,
        Back,
        FrontAndBack,
    };
    enum class FrontFace {
        CounterClockwise,
        Clockwise,
    };
    struct RasterizationState final {
        PolygonMode polygon_mode = PolygonMode::Fill;
        CullMode cull_mode = CullMode::Back;
        FrontFace front_face = FrontFace::CounterClockwise;
        bool depth_clamp_enable = false;
        bool rasterizer_discard_enable = false;
        bool depth_bias_enable = false;
        f32 depth_bias_constant_factor = 0.0f;
        f32 depth_bias_clamp = 0.0f;
        f32 depth_bias_slope_factor = 0.0f;
    };

    struct MultisampleState final {
        SampleCount rasterization_samples = SampleCount::x1;
        bool sample_shading_enable = false;
        f32 min_sample_shading = 1.0f;
        bool alpha_to_coverage_enable = false;
        bool alpha_to_one_enable = false;
    };

    enum class CompareOp {
        Never,
        Less,
        Equal,
        LessOrEqual,
        Greater,
        NotEqual,
        GreaterOrEqual,
        Always,
    };
    enum class StencilOp {
        Keep,
        Zero,
        Replace,
        IncrementAndClamp,
        DecrementAndClamp,
        Invert,
        IncrementAndWrap,
        DecrementAndWrap,
    };
    struct StencilOpState final {
        StencilOp fail_op = StencilOp::Keep;
        StencilOp pass_op = StencilOp::Keep;
        StencilOp depth_fail_op = StencilOp::Keep;
        CompareOp compare_op = CompareOp::Always;
        u32 compare_mask = 0xFF;
        u32 write_mask = 0xFF;
        u32 reference = 0;
    };
    struct DepthStencilState final {
        bool depth_test_enable = true;
        bool depth_write_enable = true;
        CompareOp depth_compare_op = CompareOp::Less;
        bool depth_bounds_test_enable = false;
        f32 min_depth_bounds = 0.0f;
        f32 max_depth_bounds = 1.0f;
        bool stencil_test_enable = false;
        StencilOpState front;
        StencilOpState back;
    };

    enum class BlendFactor {
        Zero,
        One,
        SrcColor,
        OneMinusSrcColor,
        DstColor,
        OneMinusDstColor,
        SrcAlpha,
        OneMinusSrcAlpha,
        DstAlpha,
        OneMinusDstAlpha,
        ConstantColor,
        OneMinusConstantColor,
        ConstantAlpha,
        OneMinusConstantAlpha,
        SrcAlphaSaturate,
        Src1Color,
        OneMinusSrc1Color,
        Src1Alpha,
        OneMinusSrc1Alpha,
    };
    enum class BlendOp{
        Add,
        Subtract,
        ReverseSubtract,
        Min,
        Max,
    };
    enum class LogicOp {
        Clear,
        And,
        AndReverse,
        Copy,
        AndInverted,
        NoOp,
        Xor,
        Or,
        Nor,
        Equivalent,
        Invert,
        OrReverse,
        CopyInverted,
        OrInverted,
        Nand,
        Set,
    };
    struct ColorBlendAttachment final {
        bool blend_enable = false;
        BlendFactor src_color_blend_factor = BlendFactor::One;
        BlendFactor dst_color_blend_factor = BlendFactor::Zero;
        BlendOp color_blend_op = BlendOp::Add;
        BlendFactor src_alpha_blend_factor = BlendFactor::One;
        BlendFactor dst_alpha_blend_factor = BlendFactor::Zero;
        BlendOp alpha_blend_op = BlendOp::Add;
        u8 color_write_mask = 0xF;
    };
    struct ColorBlendState final {
        bool logic_op_enable = false;
        LogicOp logic_op = LogicOp::Copy;
        std::span<const ColorBlendAttachment> attachments;
        std::array<f32, 4> blend_constants = {0.0f, 0.0f, 0.0f, 0.0f};
    };

    struct PushConstantsRange final {
        ShaderStage stage{};
        u32 offset = 0;
        u32 size = 0;
    };

    struct GraphicsPipelineDesc final {
        std::span<const ShaderModule> shaders;
        VertexInputState vertex_input;
        InputAssemblyState input_assembly;
        TessellationState tessellation;
        RasterizationState rasterization;
        MultisampleState multisample;
        DepthStencilState depth_stencil;
        ColorBlendState color_blend;

        std::span<const ImageFormat> color_formats;
        ImageFormat depth_format = ImageFormat::Undefined;
        ImageFormat stencil_format = ImageFormat::Undefined;

        std::span<PushConstantsRange> push_constants;
    };


    struct ComputePipelineDesc final {
        ShaderModule shader;
        u32 push_constant_size = 0;
    };

    enum class Filter {
        Nearest,
        Linear,
    };
    enum class AddressMode {
        Repeat,
        ClampToEdge,
        ClampToBorder,
    };
    struct SamplerDesc final {
        Filter min_filter = Filter::Linear;
        Filter mag_filter = Filter::Linear;
        Filter mip_filter = Filter::Linear;
        AddressMode address_u = AddressMode::Repeat;
        AddressMode address_v = AddressMode::Repeat;
        f32 max_anisotropy = 1.0f;
        f32 min_lod = 0.0f;
        f32 max_lod = 1.0f;
    };

    enum class AttachmentLoad { Clear, Load, DontCare };
    enum class AttachmentStore { Store, DontCare };

    struct RGColorAttachment final {
        RGImageHandle image = {};
        AttachmentLoad load = AttachmentLoad::Clear;
        AttachmentStore store = AttachmentStore::Store;
        f32 clear_r = 0.0f;
        f32 clear_g = 0.0f;
        f32 clear_b = 0.0f;
        f32 clear_a = 1.0f;
    };

    struct RGDepthAttachment final {
        RGImageHandle image = {};
        AttachmentLoad load = AttachmentLoad::Clear;
        AttachmentStore store = AttachmentStore::DontCare;
        f32 clear_value = 1.0f;
    };

    struct RGRenderingInfo final {
        std::span<RGColorAttachment> colors = {};
        RGDepthAttachment depth = {};
        u32 width = 0;
        u32 height = 0;
    };

    struct DrawInfo final {
        u32 vertex_count = 0;
        u32 instance_count = 1;
        u32 first_vertex = 0;
        u32 first_instance = 0;
    };

    struct DrawIndexedInfo final {
        u32 index_count = 0;
        u32 instance_count = 1;
        u32 first_index = 0;
        i32 vertex_offset = 0;
        u32 first_instance = 0;
    };

    struct DispatchInfo final {
        u32 x = 1;
        u32 y = 1;
        u32 z = 1;
    };

    struct RGBufferCopyInfo final {
        RGBufferHandle src = {};
        RGBufferHandle dst = {};
        usize src_offset = 0;
        usize dst_offset = 0;
        usize size = 0;
    };

    struct RGBufferImageCopyInfo final {
        RGBufferHandle src = {};
        RGImageHandle dst = {};
        usize buffer_offset = 0;
        u32 mip_level = 0;
    };

} // namespace mantle
