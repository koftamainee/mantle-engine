#pragma once

#include "core/types.h"
#include "renderer/gpu_resource_manager.h"

namespace mantle {

    enum class ImageLayout {
        Undefined,
        General,
        ColorAttachment,
        DepthAttachment,
        ShaderReadOnly,
        TransferSrc,
        TransferDst,
        Present,
    };

    enum class PipelineStage {
        Top,
        ColorOutput,
        EarlyFragmentTests,
        LateFragmentTests,
        FragmentShader,
        VertexShader,
        Transfer,
        Bottom,
    };

    struct ImageBarrier final {
        ImageHandle image;
        ImageLayout from;
        ImageLayout to;
        PipelineStage src_stage;
        PipelineStage dst_stage;
    };

    struct ColorAttachment final {
        ImageHandle image;
        ImageLayout layout;
        f32 clear_r = 0.0f;
        f32 clear_g = 0.0f;
        f32 clear_b = 0.0f;
        f32 clear_a = 1.0f;
        bool clear = true;
    };

    struct DepthAttachment final {
        ImageHandle image;
        ImageLayout layout;
        f32 clear_value = 1.0f;
        bool clear = true;
    };

    struct RenderingInfo final {
        ColorAttachment color;
        DepthAttachment depth;
        u32 width;
        u32 height;
    };

    struct DrawInfo final {
        u32 vertex_count;
        u32 instance_count = 1;
        u32 first_vertex = 0;
        u32 first_instance = 0;
    };

    struct DrawIndexedInfo final {
        u32 index_count;
        u32 instance_count = 1;
        u32 first_index = 0;
        i32 vertex_offset = 0;
        u32 first_instance = 0;
    };

    class CommandRecorder final {
      public:
        CommandRecorder() = default;
        ~CommandRecorder();

        CommandRecorder(const CommandRecorder &other) = delete;
        CommandRecorder(CommandRecorder &&other) noexcept = delete;
        CommandRecorder &operator=(const CommandRecorder &other) = delete;
        CommandRecorder &operator=(CommandRecorder &&other) noexcept = delete;

        void init();
        void destroy();

        void image_barrier(const ImageBarrier &barrier);
        void pipeline_barriers(std::span<const ImageBarrier> barriers);

        void begin_rendering(const RenderingInfo &info);
        void end_rendering();

        void bind_graphics_pipeline(GraphicsPipelineHandle pipeline);
        void bind_compute_pipeline(ComputePipelineHandle pipeline);

        void set_viewport(f32 x, f32 y, f32 width, f32 height);
        void set_scissor(i32 x, i32 y, u32 width, u32 height);

        void draw(const DrawInfo &info);
        void draw_indexed(const DrawIndexedInfo &info);

        void dispatch(u32 x, u32 y, u32 z);

        void copy_buffer(BufferHandle src, BufferHandle dst, usize size,
                         usize src_offset = 0, usize dst_offset = 0);
        void copy_buffer_to_image(BufferHandle src, ImageHandle dst, u32 width,
                                  u32 height);

        void push_constants(const void *data, u32 size);

        void bind_descriptor_set();

      private:
        // TODO
    };

} // namespace mantle
