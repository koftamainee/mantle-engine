#include "vulkan/command_recorder.h"
#include "renderer/render_graph.h"
#include "resources/gpu_resource_manager_internal.h"

namespace mantle {
    struct RenderPassContext::Impl final {
        CommandRecorder *cmd;

        GPUResourceManager *resource_manager = nullptr;
    };

    void RenderPassContext::bind_pipeline(GraphicsPipelineHandle pipeline) {
        m_impl->cmd->bind_graphics_pipeline(
            m_impl->resource_manager->m_impl->get_graphics_pipeline(pipeline));
    }

    void RenderPassContext::bind_pipeline(ComputePipelineHandle pipeline) {
        m_impl->cmd->bind_compute_pipeline(
            m_impl->resource_manager->m_impl->get_compute_pipeline(pipeline));
    }

    void RenderPassContext::begin_rendering(const RGRenderingInfo &info) {
        // TODO: use custom alloc
        std::pmr::vector<ColorAttachment> color_attachments;
        color_attachments.reserve(info.colors.size());

        for (const auto &rg : info.colors) {
            ImageResource stub{};
            ColorAttachment c = {
                .image = &stub, // FIXME
                .load = rg.load,
                .store = rg.store,
                .clear_r = rg.clear_r,
                .clear_g = rg.clear_g,
                .clear_b = rg.clear_b,
                .clear_a = rg.clear_a,
            };

            color_attachments.push_back(c);
        }

        DepthAttachment depth = {};
        if (info.depth != nullptr) {
            ImageResource stub;
            depth.image = &stub; // FIXME
            depth.load = info.depth->load;
            depth.store = info.depth->store;
            depth.clear_value = info.depth->clear_value;
        }


        RenderingInfo internal_info = {
            .colors = std::span(color_attachments.data(),
                                color_attachments.size()),
            .depth = ((info.depth != nullptr) ? &depth : nullptr),
            .width = info.width,
            .height = info.height,
        };

        m_impl->cmd->begin_rendering(internal_info);
    }

    void RenderPassContext::end_rendering() { m_impl->cmd->end_rendering(); }

    void RenderPassContext::set_viewport(f32 x, f32 y, f32 width, f32 height) {
        m_impl->cmd->set_viewport(x, y, width, height);
    }

    void RenderPassContext::set_scissor(i32 x, i32 y, u32 width, u32 height) {
        m_impl->cmd->set_scissor(x, y, width, height);
    }

    void RenderPassContext::bind_vertex_buffer(RGBufferHandle buffer,
                                               u32 binding, usize offset) {
        BufferResource buffer_internal = {}; // FIXME
        m_impl->cmd->bind_vertex_buffer(buffer_internal, binding, offset);
    }

    void RenderPassContext::bind_index_buffer(RGBufferHandle buffer,
                                              usize offset) {
        BufferResource buffer_internal = {}; // FIXME
        m_impl->cmd->bind_index_buffer(buffer_internal, offset);
    }

    void
    RenderPassContext::copy_image_to_buffer(const RGImageBufferCopyInfo &info) {
    }

    void RenderPassContext::copy_image(const RGImageCopyInfo &info) {}

    void RenderPassContext::blit_image(const RGImageBlitInfo &info) {}

    void RenderPassContext::clear_color_image(RGImageHandle image, f32 r, f32 g,
                                              f32 b, f32 a) {}

    void RenderPassContext::clear_depth_image(RGImageHandle image, f32 depth) {}

    void RenderPassContext::push_constants(const void *data,
                                           ShaderStage stage) {}

    void RenderPassContext::init(Impl *impl) {
        m_impl = impl;
    }

    void RenderPassContext::draw(const RGDrawInfo &info) {}

    void RenderPassContext::draw_indexed(const RGDrawIndexedInfo &info) {}

    void RenderPassContext::dispatch(const RGDispatchInfo &info) {}

    void RenderPassContext::copy_buffer(const RGBufferCopyInfo &info) {}

    void
    RenderPassContext::copy_buffer_to_image(const RGBufferImageCopyInfo &info) {
    }

} // namespace mantle
