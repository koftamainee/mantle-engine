#include "world/chunk_rendering_system.h"
#include "world/chunk_mesh_data.h"

#include <array>
#include <cstring>
#include "core/assert.h"
#include "core/memory/arena_allocator.h"
#include "core/memory/pmr/arena_resource.h"
#include "core/memory/scope_arena.h"
#include "helpers.h"
#include "math/aabb.h"
#include "math/frustum.h"
#include "renderer/blackboard_types.h"
#include "renderer/gpu_resource_manager.h"
#include "renderer/renderer.h"
#include "renderer/utils.h"
#include "spdlog/spdlog.h"

namespace mantle {

    static constexpr f32 VOXEL_SCALE = 0.1f;
    static constexpr f32 CHUNK_WORLD_SIZE = static_cast<f32>(Chunk::size) * VOXEL_SCALE;

    ChunkRenderingSystem::~ChunkRenderingSystem() { destroy(); }

    void ChunkRenderingSystem::init(Renderer &renderer,
                                    ArenaAllocator &scratch_arena,
                                    u32 max_chunks) {
        MANTLE_CHECK(!m_is_initialized);

        m_logger = spdlog::get("world").get();
        m_renderer = &renderer;
        m_max_chunks = max_chunks;
        auto &rm = renderer.resource_manager();

        m_slots = scratch_arena.push<ChunkMeshSlot>(max_chunks);
        std::memset(m_slots, 0, sizeof(ChunkMeshSlot) * max_chunks);

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

        VertexBinding bindings[] = {{0, 16}};
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

        m_vertex_stride = ChunkMeshingSystem::MAX_QUADS_PER_CHUNK * 4 * sizeof(MeshVertex);
        m_index_stride = ChunkMeshingSystem::MAX_QUADS_PER_CHUNK * 6 * sizeof(u32);

        m_vertex_buffer = rm.create_buffer(
            {
                .size = static_cast<u64>(max_chunks) * m_vertex_stride,
                .usage = BufferUsage::Vertex | BufferUsage::Transfer,
                .memory = MemoryType::CpuToGpu,
            },
            true);

        m_index_buffer = rm.create_buffer(
            {
                .size = static_cast<u64>(max_chunks) * m_index_stride,
                .usage = BufferUsage::Index | BufferUsage::Transfer,
                .memory = MemoryType::CpuToGpu,
            },
            true);

        for (u32 i = 0; i < max_chunks; i++) {
            m_slots[i].vertex_offset = i * m_vertex_stride;
            m_slots[i].index_offset = i * m_index_stride;
        }

        m_is_initialized = true;
        m_logger->info(
            "Chunk rendering system initialized ({} chunks, {} MB total)",
            max_chunks,
            (static_cast<u64>(max_chunks) * (m_vertex_stride + m_index_stride)) /
                (1024 * 1024));
    }

    void ChunkRenderingSystem::destroy() {
        if (m_is_initialized) {
            auto &rm = m_renderer->resource_manager();
            rm.destroy_buffer(m_vertex_buffer);
            rm.destroy_buffer(m_index_buffer);
            rm.destroy_graphics_pipeline(m_mesh_pipeline);
            m_is_initialized = false;
            m_logger->info("Chunk rendering system destroyed");
        }
    }

    void ChunkRenderingSystem::add_passes(FrameGraph &graph,
                                          const Blackboard &blackboard) const {
        MANTLE_CHECK(m_is_initialized);

        auto backbuffer = blackboard.get<BbBackbuffer>().handle;
        const auto &camera_data = blackboard.get<BbCameraData>();
        const auto &fb_size = blackboard.get<BbFramebufferSize>();

        u32 width = fb_size.width;
        u32 height = fb_size.height;

        FGBufferHandle vertex_fg = graph.import_buffer(m_vertex_buffer);
        FGBufferHandle index_fg = graph.import_buffer(m_index_buffer);

        struct RenderPass {
            FGBufferHandle vertex;
            FGBufferHandle index;
            FGImageHandle color;
            FGImageHandle depth;
            glm::mat4 camera_data;
        };

        graph.add_pass<RenderPass>(
            "Render Chunk Meshes",
            [&](FrameGraphBuilder &builder, RenderPass &pass) {
                pass.depth = builder.create_image({
                    .width = width,
                    .height = height,
                    .format = ImageFormat::D32,
                    .usage = ImageUsage::Depth,
                });
                pass.vertex = builder.read(vertex_fg, BufferReadUsage::Vertex);
                pass.index = builder.read(index_fg, BufferReadUsage::Index);
                pass.color =
                    builder.write(backbuffer, WriteUsage::ColorAttachment);
                builder.write(pass.depth, WriteUsage::DepthAttachment);
                pass.camera_data = camera_data.view_proj;
            },
            [this, width, height](FGPassContext &ctx,
                                               const RenderPass &pass) {
                Frustum frustum;
                frustum.extract(pass.camera_data);

                std::array<FGColorAttachment, 1> colors = {{
                    {
                        .image = pass.color,
                        .load = AttachmentLoad::Clear,
                        .store = AttachmentStore::Store,
                    },
                }};
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
                ctx.push_constants(&pass.camera_data, ShaderStage::Vertex);

                for (u32 i = 0; i < m_max_chunks; i++) {
                    const auto &slot = m_slots[i];
                    if (slot.quad_count == 0)
                        continue;

                    glm::vec3 base =
                        glm::vec3(static_cast<f32>(slot.position_x), static_cast<f32>(slot.position_y),
                                  static_cast<f32>(slot.position_z)) *
                        CHUNK_WORLD_SIZE;
                    AABB aabb{base, base + CHUNK_WORLD_SIZE};
                    if (!frustum.intersects(aabb)) {
                        continue;
                    }

                    ctx.bind_vertex_buffer(pass.vertex, 0, slot.vertex_offset);
                    ctx.bind_index_buffer(pass.index, slot.index_offset);
                    ctx.draw_indexed({slot.quad_count * 6, 1, 0, 0, 0});
                }

                ctx.end_rendering();
            });
    }

} // namespace mantle
