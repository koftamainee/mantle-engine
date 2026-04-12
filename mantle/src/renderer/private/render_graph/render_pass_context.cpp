#include "renderer/render_graph.h"

// TODO
namespace mantle {
    RenderPassContext::~RenderPassContext() {}

    void RenderPassContext::bind_pipeline(GraphicsPipelineHandle pipeline) {}

    void RenderPassContext::bind_pipeline(ComputePipelineHandle pipeline) {}

    void RenderPassContext::draw(u32 vertex_count, u32 instance_count,
                                 u32 first_vertex, u32 first_instance) {}

    void RenderPassContext::draw_indexed(u32 index_count, u32 instance_count,
                                         u32 first_index, i32 vertex_offset,
                                         u32 first_instance) {}

    void RenderPassContext::draw_indirect(RGBufferHandle indirect_buffer,
                                          u32 offset, u32 draw_count) {}

    void
    RenderPassContext::draw_indexed_indirect(RGBufferHandle indirect_buffer,
                                             u32 offset, u32 draw_count) {}

    void RenderPassContext::dispatch(u32 x, u32 y, u32 z) {}

    void RenderPassContext::push_constants(const void *data, u32 size) {}

} // namespace mantle
