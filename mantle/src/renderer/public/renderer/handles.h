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
        Rgba16,
        Rgba32,
        R32,
        Rg16,
        D32,
        D24S8,
    };
    enum class ImageUsage {
        Storage = 1 << 0,
        Sampled = 1 << 1,
        Color = 1 << 2,
        Depth = 1 << 3,
        TransferSrc = 1 << 4,
        TransferDst = 1 << 5,
        MaxEnum = (1 << 6) - 1,
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
} // namespace mantle
