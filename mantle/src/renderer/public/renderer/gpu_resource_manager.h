#pragma once
#include <vector>

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

    class GPUResourceManager final {
      public:
        GPUResourceManager() = delete;
        ~GPUResourceManager();

        GPUResourceManager(const GPUResourceManager &other) = delete;
        GPUResourceManager(GPUResourceManager &&other) noexcept = delete;
        GPUResourceManager &operator=(const GPUResourceManager &other) = delete;
        GPUResourceManager &
        operator=(GPUResourceManager &&other) noexcept = delete;

        ShaderHandle create_shader(std::pmr::vector<u32> spir_v);
        void destroy_shader(ShaderHandle shader);

        GraphicsPipelineHandle
        create_graphics_pipeline(const GraphicsPipelineDesc &desc);
        ComputePipelineHandle
        create_compute_pipeline(const ComputePipelineDesc &desc);
        void destroy_graphics_pipeline(GraphicsPipelineHandle pipeline);
        void destroy_compute_pipeline(ComputePipelineHandle pipeline);

        BufferHandle create_buffer(const BufferDesc &desc);
        void destroy_buffer(BufferHandle buffer);

        ImageHandle create_image(const ImageDesc &desc);
        void destroy_image(ImageHandle image);

      private:
        // TODO
    };
} // namespace mantle
