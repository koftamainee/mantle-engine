#include "renderer/render_graph.h"

// TODO
namespace mantle {
    RenderPassContext::~RenderPassContext() {}

    void RenderPassContext::bind_pipeline(GraphicsPipelineHandle pipeline) {}

    void RenderPassContext::bind_pipeline(ComputePipelineHandle pipeline) {}

    void RenderPassContext::begin_rendering(const RGRenderingInfo &info) {}

    void RenderPassContext::end_rendering() {}

    void RenderPassContext::draw(const RGDrawInfo &info) {}

    void RenderPassContext::draw_indexed(const RGDrawIndexedInfo &info) {}

    void RenderPassContext::dispatch(const RGDispatchInfo &info) {}

    void RenderPassContext::copy_buffer(const RGBufferCopyInfo &info) {}

    void
    RenderPassContext::copy_buffer_to_image(const RGBufferImageCopyInfo &info) {
    }

    void RenderPassContext::push_constants(const void *data, u32 size, u32 offset) {}

} // namespace mantle
