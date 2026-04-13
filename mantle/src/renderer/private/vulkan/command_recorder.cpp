#include "command_recorder.h"

// TODO
namespace mantle {
    CommandRecorder::~CommandRecorder() {}

    void CommandRecorder::init() {}

    void CommandRecorder::destroy() {}

    void CommandRecorder::image_barrier(const ImageBarrier &barrier) {}

    void
    CommandRecorder::pipeline_barriers(std::span<const ImageBarrier> barriers) {
    }

    void CommandRecorder::begin_rendering(const RenderingInfo &info) {}

    void CommandRecorder::end_rendering() {}

    void
    CommandRecorder::bind_graphics_pipeline(GraphicsPipelineHandle pipeline) {}

    void
    CommandRecorder::bind_compute_pipeline(ComputePipelineHandle pipeline) {}

    void CommandRecorder::set_viewport(f32 x, f32 y, f32 width, f32 height) {}

    void CommandRecorder::set_scissor(i32 x, i32 y, u32 width, u32 height) {}

    void CommandRecorder::draw(const DrawInfo &info) {}

    void CommandRecorder::draw_indexed(const DrawIndexedInfo &info) {}

    void CommandRecorder::dispatch(u32 x, u32 y, u32 z) {}

    void CommandRecorder::copy_buffer(BufferHandle src, BufferHandle dst,
                                      usize size, usize src_offset,
                                      usize dst_offset) {}

    void CommandRecorder::copy_buffer_to_image(BufferHandle src,
                                               ImageHandle dst, u32 width,
                                               u32 height) {}

    void CommandRecorder::push_constants(const void *data, u32 size) {}

    void CommandRecorder::bind_descriptor_set() {}
} // namespace mantle
