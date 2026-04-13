#pragma once
#include "core/enum_flags.h"
#include "core/types.h"

namespace mantle {
    struct BufferHandle final {
        u32 index;
        u32 generation;
    };
    struct ImageHandle final {
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

    enum class BufferUsage {
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
        usize size;
        BufferUsage usage;
        MemoryType memory;
    };

    enum class ImageFormat {
        Rgba8,
        Rgba8Srgb,
        Bgra8,
        Bgra8Srgb,
        Rgba16,
        Rgba32,
        Rg16,
        R32,
        D16,
        D24S8,
        D32,
        D32S8,
    };

    enum class ImageUsage : u32 {
        None = 0,

        Sampled = 1 << 0,
        Storage = 1 << 1,

        Color = 1 << 2,
        Depth = 1 << 3,
        Stencil = 1 << 4,

        TransferSrc = 1 << 5,
        TransferDst = 1 << 6,

        MaxEnum = (1 << 7) - 1
    };
    struct ImageDesc final {
        u32 width = 0;
        u32 height = 0;
        u32 depth = 1;
        u32 mip_levels = 1;
        u32 array_layers = 1;
        u32 sample_count = 1;
        ImageFormat format{};
        ImageUsage usage{};
        bool create_view = true;
    };

    struct GraphicsPipelineDesc final {}; // TODO
    struct ComputePipelineDesc final {}; // TODO

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

    struct SamplerHandle final {
        u32 index;
        u32 generation;
    };

    inline static constexpr u32 RemainingMipLevels = ~0u;

    enum class ImageLayout {
        Undefined,
        General,
        ColorAttachment,
        DepthAttachment,
        AttachmentOptimal,
        ShaderReadOnly,
        ReadOnlyOptimal,
        TransferSrc,
        TransferDst,
        Present,
    };

    enum class PipelineStage {
        None,
        Top,
        Bottom,
        AllCommands,
        AllGraphics,

        VertexInput,
        VertexShader,
        EarlyFragmentTests,
        FragmentShader,
        LateFragmentTests,
        ColorOutput,

        ComputeShader,

        Transfer,
        Blit,
        Copy,
        Resolve,
        Clear,
    };

    struct ImageBarrier final {
        ImageHandle image{};
        ImageLayout from{};
        ImageLayout to{};
        PipelineStage src_stage{};
        PipelineStage dst_stage{};
        u32 base_mip = 0;
        u32 mip_count = RemainingMipLevels;
    };

    enum class AttachmentLoad { Clear, Load, DontCare };
    enum class AttachmentStore { Store, DontCare };
    struct ColorAttachment final {
        ImageHandle image{};
        ImageLayout layout{};
        AttachmentLoad load = AttachmentLoad::Clear;
        AttachmentStore store = AttachmentStore::Store;
        f32 clear_r = 0.0f;
        f32 clear_g = 0.0f;
        f32 clear_b = 0.0f;
        f32 clear_a = 1.0f;
        bool clear = true;
    };

    struct DepthAttachment final {
        ImageHandle image{};
        ImageLayout layout{};
        f32 clear_value = 1.0f;
        bool clear = true;
    };

    struct RenderingInfo final {
        ColorAttachment color{};
        DepthAttachment depth{};
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

} // namespace mantle
