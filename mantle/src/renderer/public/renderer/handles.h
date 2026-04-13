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
        MaxEnum = (1 << 4) - 1,
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
        Rgba16,
        Rgba32,
        R32,
        D32,
    };
    enum class ImageUsage {
        Storage = 1 << 0,
        Sampled = 1 << 1,
        Color = 1 << 2,
        Depth = 1 << 3,
        Transfer = 1 << 4,
        MaxEnum = (1 << 5) - 1,
    };
    struct ImageDesc final {
        u32 width;
        u32 height;
        u32 depth;
        ImageFormat format;
        ImageUsage usage;
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
        bool anisotropy = false;
    };

    struct SamplerHandle final {
        u32 index;
        u32 generation;
    };
} // namespace mantle
