#include "render_graph/render_pass_context_internal.h"

namespace mantle {
    void RenderPassContext::bind_pipeline(GraphicsPipelineHandle pipeline) {
        m_impl->cmd->bind_graphics_pipeline(
            m_impl->resource_manager->m_impl->get_graphics_pipeline(pipeline));
    }

    void RenderPassContext::bind_pipeline(ComputePipelineHandle pipeline) {
        m_impl->cmd->bind_compute_pipeline(
            m_impl->resource_manager->m_impl->get_compute_pipeline(pipeline));
    }

    void RenderPassContext::begin_rendering(const RGRenderingInfo &info) {
        ScopeArena scope(m_impl->scratch_arena);
        std::pmr::vector<ColorAttachment> color_attachments(
            m_impl->scratch_resource);
        color_attachments.reserve(info.colors.size());

        for (const auto &rg : info.colors) {

            ImageHandle image_handle =
                m_impl->transient_resources->get_image(rg.image);
            ImageResource &image_resource =
                m_impl->resource_manager->m_impl->get_image(image_handle);

            ColorAttachment c = {
                .image = &image_resource,
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

            ImageHandle depth_image_handle =
                m_impl->transient_resources->get_image(info.depth->image);
            ImageResource &depth_image_resource =
                m_impl->resource_manager->m_impl->get_image(depth_image_handle);

            depth.image = &depth_image_resource, depth.load = info.depth->load;
            depth.store = info.depth->store;
            depth.clear_value = info.depth->clear_value;
        }


        RenderingInfo internal_info = {
            .colors =
                std::span(color_attachments.data(), color_attachments.size()),
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
        BufferHandle buffer_handle =
            m_impl->transient_resources->get_buffer(buffer);
        BufferResource &buffer_resource =
            m_impl->resource_manager->m_impl->get_buffer(buffer_handle);

        m_impl->cmd->bind_vertex_buffer(buffer_resource, binding, offset);
    }

    void RenderPassContext::bind_index_buffer(RGBufferHandle buffer,
                                              usize offset) {
        BufferHandle buffer_handle =
    m_impl->transient_resources->get_buffer(buffer);
        BufferResource &buffer_resource =
            m_impl->resource_manager->m_impl->get_buffer(buffer_handle);

        m_impl->cmd->bind_index_buffer(buffer_resource, offset);
    }

    void
    RenderPassContext::copy_image_to_buffer(const RGImageBufferCopyInfo &info) {
        BufferHandle buf = m_impl->transient_resources->get_buffer(info.dst);
        BufferResource &dst = m_impl->resource_manager->m_impl->get_buffer(buf);

        ImageHandle img =  m_impl->transient_resources->get_image(info.src);
        ImageResource &src = m_impl->resource_manager->m_impl->get_image(img);

        ImageBufferCopyInfo info_internal = {
            .src = &src,
            .dst = &dst,
            .buffer_offset = info.buffer_offset,
            .mip_level = info.mip_level,
        };

        m_impl->cmd->copy_image_to_buffer(info_internal);
    }

    void RenderPassContext::copy_image(const RGImageCopyInfo &info) {
        ImageHandle dst_handle = m_impl->transient_resources->get_image(info.dst);
        ImageResource &dst = m_impl->resource_manager->m_impl->get_image(dst_handle);

        ImageHandle src_handle = m_impl->transient_resources->get_image(info.src);
        ImageResource &src = m_impl->resource_manager->m_impl->get_image(src_handle);

        ImageCopyInfo info_internal = {
            .src = &src,
            .dst = &dst,
            .src_mip_level = info.src_mip_level,
            .dst_mip_level = info.dst_mip_level,
            .src_array_layer = info.src_array_layer,
            .dst_array_layer = info.dst_array_layer,
            .width = info.width,
            .height = info.height,
        };

        m_impl->cmd->copy_image(info_internal);
    }

    void RenderPassContext::blit_image(const RGImageBlitInfo &info) {
        ImageHandle dst_handle = m_impl->transient_resources->get_image(info.dst);
        ImageResource &dst = m_impl->resource_manager->m_impl->get_image(dst_handle);

        ImageHandle src_handle = m_impl->transient_resources->get_image(info.src);
        ImageResource &src = m_impl->resource_manager->m_impl->get_image(src_handle);

        ImageBlitInfo info_internal = {
            .src = &src,
            .dst = &dst,
            .src_region = info.src_region,
            .dst_region = info.dst_region,
            .filter = info.filter,
        };

        m_impl->cmd->blit_image(info_internal);
    }

    void RenderPassContext::clear_color_image(RGImageHandle image, f32 r, f32 g,
                                              f32 b, f32 a) {
        ImageHandle handle = m_impl->transient_resources->get_image(image);
        ImageResource &resource = m_impl->resource_manager->m_impl->get_image(handle);
        m_impl->cmd->clear_color_image(resource, r, g, b, a);
    }

    void RenderPassContext::clear_depth_image(RGImageHandle image, f32 depth) {
        ImageHandle handle = m_impl->transient_resources->get_image(image);
        ImageResource &resource = m_impl->resource_manager->m_impl->get_image(handle);
        m_impl->cmd->clear_depth_image(resource, depth);
    }

    void RenderPassContext::push_constants(const void *data,
                                           ShaderStage stage) {
        m_impl->cmd->push_constants(data, stage);
    }

    void RenderPassContext::init(Impl *impl) { m_impl = impl; }

    void RenderPassContext::draw(const RGDrawInfo &info) {
        m_impl->cmd->draw(info);
    }

    void RenderPassContext::draw_indexed(const RGDrawIndexedInfo &info) {
        m_impl->cmd->draw_indexed(info);
    }

    void RenderPassContext::dispatch(const RGDispatchInfo &info) {
        m_impl->cmd->dispatch(info);
    }

    void RenderPassContext::copy_buffer(const RGBufferCopyInfo &info) {
        BufferHandle dst_handle = m_impl->transient_resources->get_buffer(info.dst);
        BufferResource &dst = m_impl->resource_manager->m_impl->get_buffer(dst_handle);

        BufferHandle src_handle = m_impl->transient_resources->get_buffer(info.src);
        BufferResource &src = m_impl->resource_manager->m_impl->get_buffer(src_handle);

        BufferCopyInfo info_internal = {
            .src = &src,
            .dst = &dst,
            .src_offset = info.src_offset,
            .dst_offset = info.dst_offset,
            .size = info.size,
        };

        m_impl->cmd->copy_buffer(info_internal);
    }

    void
    RenderPassContext::copy_buffer_to_image(const RGBufferImageCopyInfo &info) {
        ImageHandle dst_handle = m_impl->transient_resources->get_image(info.dst);
        ImageResource &dst = m_impl->resource_manager->m_impl->get_image(dst_handle);

        BufferHandle src_handle = m_impl->transient_resources->get_buffer(info.src);
        BufferResource &src = m_impl->resource_manager->m_impl->get_buffer(src_handle);

        BufferImageCopyInfo info_internal = {
            .src = &src,
            .dst = &dst,
            .buffer_offset = info.buffer_offset,
            .mip_level = info.mip_level,
        };

        m_impl->cmd->copy_buffer_to_image(info_internal);
    }

} // namespace mantle
