#include "world/chunk_rendering_system.h"
#include "world/chunk_meshing_types.h"

#include "core/memory/pmr/arena_resource.h"
#include "core/memory/scope_arena.h"
#include "helpers.h"
#include "renderer/blackboard_types.h"
#include "renderer/gpu_resource_manager.h"
#include "renderer/renderer.h"
#include "renderer/utils.h"

namespace mantle {

    ChunkRenderingSystem::~ChunkRenderingSystem() { destroy(); }

    void ChunkRenderingSystem::init(Renderer &renderer,
                                 ArenaAllocator &scratch_arena) {
        check(!m_is_initialized);

        m_renderer = &renderer;
        auto &rm = renderer.resource_manager();

        ScopeArena scope(&scratch_arena);
        ArenaResource resource(&scratch_arena);

        std::pmr::vector<u32> spv(&resource);
        load_spv("assets/shaders/mesh_render.spv", spv);
        ShaderHandle shader = rm.create_shader(spv);

        ShaderModule modules[] = {
            {"vert_main", ShaderStage::Vertex, shader},
            {"frag_main", ShaderStage::Fragment, shader},
        };
        ImageFormat color_format = renderer.get_swapchain_info().surface_format;

        ColorBlendAttachment attachment = {};
        ColorBlendState blend = {};
        blend.attachments = span(attachment);

        DepthStencilState depth_stencil = {
            .depth_test_enable = true,
            .depth_write_enable = true,
            .depth_compare_op = CompareOp::Less,
        };

        VertexBinding bindings[] = {
            {0, 16},
        };
        VertexAttribute attributes[] = {
            {0, 0, VertexFormat::Float3, 0},
            {1, 0, VertexFormat::Uint1, 12},
        };

        PushConstantsRange pc[] = {
            {ShaderStage::Vertex, sizeof(glm::mat4), 0},
        };

        m_mesh_pipeline = rm.create_graphics_pipeline({
            .shaders = modules,
            .vertex_input = {bindings, attributes},
            .rasterization = {.cull_mode = CullMode::Back,
                              .front_face = FrontFace::CounterClockwise},
            .depth_stencil = depth_stencil,
            .color_blend = blend,
            .color_formats = span(color_format),
            .depth_format = ImageFormat::D32,
            .push_constants = pc,
        });
        rm.destroy_shader(shader, true);

        m_is_initialized = true;
        spdlog::info("Chunk rendering system is initialized");
    }

    void ChunkRenderingSystem::add_passes(FrameGraph &graph,
                                       Blackboard &blackboard) const {
        auto backbuffer = blackboard.get<BbBackbuffer>().handle;
        const auto& camera_data = blackboard.get<BbCameraData>();
        const auto& fb_size = blackboard.get<BbFramebufferSize>();
        const auto& vertex_fg = blackboard.get<BbChunkVertex>();
        const auto& index_fg = blackboard.get<BbChunkIndex>();
        const auto& indirect_fg = blackboard.get<BbChunkIndirect>();

        u32 width = fb_size.width;
        u32 height = fb_size.height;

        struct RenderPass final {
            FGBufferHandle vertex;
            FGBufferHandle index;
            FGBufferHandle indirect;
            FGImageHandle color;
            FGImageHandle depth;
        };
        graph.add_pass<RenderPass>(
            "Render Meshes",
            [&](FrameGraphBuilder &builder, RenderPass &pass) {
                pass.depth = builder.create_image({
                    .width = width,
                    .height = height,
                    .format = ImageFormat::D32,
                    .usage = ImageUsage::Depth,
                });
                pass.vertex =
                    builder.read(vertex_fg.handle, BufferReadUsage::Vertex);
                pass.index =
                    builder.read(index_fg.handle, BufferReadUsage::Index);
                pass.indirect = builder.read(indirect_fg.handle,
                                             BufferReadUsage::IndirectArg);
                pass.color =
                    builder.write(backbuffer, WriteUsage::ColorAttachment);
                builder.write(pass.depth, WriteUsage::DepthAttachment);
            },
            [width, height, camera_data, this](FGPassContext &ctx,
                                               const RenderPass &pass) {
                std::array<FGColorAttachment, 1> colors = {{{
                    .image = pass.color,
                    .load = AttachmentLoad::Clear,
                    .store = AttachmentStore::Store,
                }}};
                FGDepthAttachment depth_att = {
                    .image = pass.depth,
                    .load = AttachmentLoad::Clear,
                    .store = AttachmentStore::DontCare,
                    .clear_value = 1.0f,
                };
                ctx.begin_rendering({
                    .colors = colors,
                    .depth = &depth_att,
                    .width = width,
                    .height = height,
                });
                ctx.set_viewport(0, 0, static_cast<f32>(width),
                                 static_cast<f32>(height));
                ctx.set_scissor(0, 0, width, height);
                ctx.bind_pipeline(m_mesh_pipeline);

                ctx.push_constants(&camera_data.view_proj,
                                   ShaderStage::Vertex);

                ctx.bind_vertex_buffer(pass.vertex, 0);
                ctx.bind_index_buffer(pass.index);
                ctx.draw_indexed_indirect({
                    .buffer = pass.indirect,
                    .draw_count = 1,
                    .stride = 0,
                });
                ctx.end_rendering();
            });
    }

    void ChunkRenderingSystem::destroy() {
        if (m_is_initialized) {
            m_renderer->resource_manager().destroy_graphics_pipeline(
                m_mesh_pipeline);
            m_is_initialized = false;
            spdlog::info("Chunk rendering system is destroyed");
        }
    }

} // namespace mantle
