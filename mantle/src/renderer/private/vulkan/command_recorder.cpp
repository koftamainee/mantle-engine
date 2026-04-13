#include "command_recorder.h"

#include <vulkan/vulkan_core.h>

// TODO add GPUResourceManager and VulkanDevice as private fields
namespace mantle {
    void CommandRecorder::set_command_buffer(VkCommandBuffer cmd) {
        m_cmd = cmd;
    }

    void CommandRecorder::image_barrier(const ImageBarrier &barrier) {} // TODO

    void
    CommandRecorder::pipeline_barriers(std::span<const ImageBarrier> barriers) {
    } // TODO

    void CommandRecorder::begin_rendering(const RenderingInfo &info) {} //  TODO

    void CommandRecorder::end_rendering() { vkCmdEndRendering(m_cmd); }

    void
    CommandRecorder::bind_graphics_pipeline(GraphicsPipelineHandle pipeline) {} // TODO

    void
    CommandRecorder::bind_compute_pipeline(ComputePipelineHandle pipeline) {} // TODO

    void CommandRecorder::set_viewport(f32 x, f32 y, f32 width, f32 height) {
        VkViewport viewport = {
            .x = x,
            .y = y,
            .width = width,
            .height = height,
            .minDepth = 0.0f,
            .maxDepth = 1.0f,
        };
        vkCmdSetViewport(m_cmd, 0, 1, &viewport);
    }

    void CommandRecorder::set_scissor(i32 x, i32 y, u32 width, u32 height) {
        VkRect2D scissor = {
            .offset = {.x = x, .y = y},
            .extent = {.width = width, .height = height},
        };
        vkCmdSetScissor(m_cmd, 0, 1, &scissor);
    }

    void CommandRecorder::draw(const DrawInfo &info) {
        vkCmdDraw(m_cmd, info.vertex_count, info.instance_count,
                  info.first_vertex, info.first_instance);
    }

    void CommandRecorder::draw_indexed(const DrawIndexedInfo &info) {
        vkCmdDrawIndexed(m_cmd, info.index_count, info.instance_count,
                         info.first_index, info.vertex_offset,
                         info.first_instance);
    }

    void CommandRecorder::dispatch(u32 x, u32 y, u32 z) {
        vkCmdDispatch(m_cmd, x, y, z);
    }

    void CommandRecorder::copy_buffer(BufferHandle src, BufferHandle dst,
                                      usize size, usize src_offset,
                                      usize dst_offset) {} // TODO

    void CommandRecorder::copy_buffer_to_image(BufferHandle src,
                                               ImageHandle dst, u32 width,
                                               u32 height) {} // TODO

    void CommandRecorder::push_constants(const void *data, u32 size) {} // TODO

    void CommandRecorder::bind_descriptor_set() {} // TODO
} // namespace mantle
