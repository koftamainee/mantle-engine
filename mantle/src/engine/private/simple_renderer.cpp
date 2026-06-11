// Copyright (c) 2026 Mantle. All rights reserved.

#include "simple_renderer.h"

#include <array>
#include <memory_resource>
#include <span>
#include <vector>

#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "mantle/core/assert.h"
#include "mantle/core/memory/arena_allocator.h"
#include "mantle/core/memory/pmr/arena_resource.h"
#include "mantle/core/memory/scope_arena.h"
#include "mantle/ecs/components.h"
#include "mantle/renderer/blackboard_types.h"
#include "mantle/renderer/utils.h"

namespace mantle {
    namespace {
        struct BasicVertex {
            glm::vec3 position;
            glm::vec3 color;
        };

        constexpr u32 CAPSULE_SEGMENTS = 16;
        constexpr u32 CAPSULE_HEMI_SEGMENTS = 4;

        constexpr f32 CAPSULE_RADIUS = 0.3f;
        constexpr f32 CAPSULE_HALF_HEIGHT = 0.5f;

        u32 push_ring(std::pmr::vector<BasicVertex> &verts, f32 y, f32 radius, u32 segments) {
            u32 start = static_cast<u32>(verts.size());
            for (u32 i = 0; i < segments; i++) {
                f32 a = static_cast<f32>(i) / static_cast<f32>(segments) * glm::two_pi<f32>();
                glm::vec3 n = glm::normalize(glm::vec3(std::cos(a), 0.0f, -std::sin(a)));
                verts.push_back({{std::cos(a) * radius, y, -std::sin(a) * radius}, n});
            }
            return start;
        }

        void push_hemi_rings(std::pmr::vector<BasicVertex> &verts, u32 segments, f32 center_y,
                             f32 radius, f32 sign) {
            for (u32 j = 1; j <= CAPSULE_HEMI_SEGMENTS; j++) {
                f32 t = static_cast<f32>(j) / static_cast<f32>(CAPSULE_HEMI_SEGMENTS);
                f32 a = t * glm::half_pi<f32>();
                f32 r = radius * std::cos(a);
                f32 y = center_y + sign * radius * std::sin(a);
                for (u32 i = 0; i < segments; i++) {
                    f32 ang = static_cast<f32>(i) / static_cast<f32>(segments) * glm::two_pi<f32>();
                    glm::vec3 n = glm::normalize(glm::vec3(std::cos(ang), sign * std::sin(a),
                                                            -std::sin(ang)));
                    verts.push_back({{std::cos(ang) * r, y, -std::sin(ang) * r}, n});
                }
            }
        }

        void push_quad_indices(std::pmr::vector<u32> &indices, u32 a, u32 b, u32 c, u32 d) {
            indices.push_back(a);
            indices.push_back(b);
            indices.push_back(c);
            indices.push_back(a);
            indices.push_back(c);
            indices.push_back(d);
        }

    } // namespace

    void SimpleRenderer::init(Renderer &renderer, MemoryBlock block) {
        MANTLE_CHECK(!m_is_initialized);

        m_arena.init(block);
        m_logger = spdlog::get("engine").get();

        auto &rm = renderer.resource_manager();

        ScopeArena            scope(&m_arena);
        ArenaResource         resource(&m_arena);
        std::pmr::vector<u32> spv(&resource);

        load_spv("assets/shaders/basic_mesh.spv", spv);
        ShaderHandle shader = rm.create_shader(spv);

        ShaderModule modules[] = {
            {"vert_main", ShaderStage::Vertex, shader},
            {"frag_main", ShaderStage::Fragment, shader},
        };
        ImageFormat color_format = renderer.get_swapchain_info().surface_format;

        ColorBlendAttachment attachment = {};
        ColorBlendState      blend = {};
        blend.attachments = std::span(&attachment, 1);

        DepthStencilState depth_stencil = {
            .depth_test_enable = true,
            .depth_write_enable = true,
            .depth_compare_op = CompareOp::Less,
        };

        VertexBinding   bindings[] = {{0, sizeof(BasicVertex)}};
        VertexAttribute attributes[] = {
            {0, 0, VertexFormat::Float3, offsetof(BasicVertex, position)},
            {1, 0, VertexFormat::Float3, offsetof(BasicVertex, color)},
        };

        PushConstantsRange pc[] = {
            {ShaderStage::Vertex, sizeof(glm::mat4) * 2, 0},
        };

        m_pipeline = rm.create_graphics_pipeline({
            .shaders = modules,
            .vertex_input = {bindings, attributes},
            .rasterization = {.cull_mode = CullMode::None,
                              .front_face = FrontFace::CounterClockwise},
            .depth_stencil = depth_stencil,
            .color_blend = blend,
            .color_formats = std::span(&color_format, 1),
            .depth_format = ImageFormat::D32,
            .push_constants = pc,
        });
        rm.destroy_shader(shader, true);

        create_floor_mesh(renderer);
        create_capsule_mesh(renderer);

        m_is_initialized = true;
        m_logger->info("Simple renderer initialized ({} meshes)", m_mesh_count);
    }

    void SimpleRenderer::destroy(Renderer &renderer) {
        if (m_is_initialized) {
            auto &rm = renderer.resource_manager();
            for (u32 i = 0; i < m_mesh_count; i++) {
                rm.destroy_buffer(m_meshes[i].vertex_buffer);
                rm.destroy_buffer(m_meshes[i].index_buffer);
            }
            rm.destroy_graphics_pipeline(m_pipeline);
            m_is_initialized = false;
            m_logger->info("Simple renderer destroyed");
        }
    }

    u32 SimpleRenderer::add_mesh(Renderer &renderer, std::span<const u8> vertices,
                                  u32 vertex_stride, std::span<const u32> indices) {
        MANTLE_CHECK(m_mesh_count < 2);

        auto &rm = renderer.resource_manager();
        u32   idx = m_mesh_count;

        u32 vertex_bytes = static_cast<u32>(vertices.size());
        u32 index_bytes = static_cast<u32>(indices.size()) * sizeof(u32);

        m_meshes[idx].vertex_buffer = rm.create_buffer(
            {
                .size = vertex_bytes,
                .usage = BufferUsage::Vertex | BufferUsage::Transfer,
                .memory = MemoryType::CpuToGpu,
            },
            true);
        rm.update_buffer(m_meshes[idx].vertex_buffer, vertices.data(), vertex_bytes);

        m_meshes[idx].index_buffer = rm.create_buffer(
            {
                .size = index_bytes,
                .usage = BufferUsage::Index | BufferUsage::Transfer,
                .memory = MemoryType::CpuToGpu,
            },
            true);
        rm.update_buffer(m_meshes[idx].index_buffer, indices.data(), index_bytes);

        m_meshes[idx].vertex_count = vertex_bytes / vertex_stride;
        m_meshes[idx].index_count = static_cast<u32>(indices.size());
        m_meshes[idx].vertex_stride = vertex_stride;

        m_mesh_count++;
        return idx;
    }

    void SimpleRenderer::create_floor_mesh(Renderer &renderer) {
        ArenaResource               resource(&m_arena);
        std::pmr::vector<BasicVertex> verts(&resource);
        std::pmr::vector<u32>         inds(&resource);

        constexpr f32 hx = 10.0f;
        constexpr f32 hz = 10.0f;
        constexpr f32 hy = 0.5f;
        constexpr glm::vec3 base_color{0.3f, 0.5f, 0.3f};

        auto push_face = [&](glm::vec3 a, glm::vec3 b, glm::vec3 c, glm::vec3 d,
                             glm::vec3 n) {
            u32 start = static_cast<u32>(verts.size());
            f32 bright = 0.8f + 0.2f * (n.y * 0.5f + 0.5f);
            glm::vec3 col = base_color * bright;
            for (auto &p : {a, b, c, d}) {
                verts.push_back({p, col});
            }
            push_quad_indices(inds, start, start + 1, start + 2, start + 3);
        };

        push_face({-hx, hy, -hz}, {hx, hy, -hz}, {hx, hy, hz}, {-hx, hy, hz}, {0, 1, 0});
        push_face({-hx, -hy, hz}, {hx, -hy, hz}, {hx, -hy, -hz}, {-hx, -hy, -hz}, {0, -1, 0});
        push_face({-hx, -hy, hz}, {hx, -hy, hz}, {hx, hy, hz}, {-hx, hy, hz}, {0, 0, 1});
        push_face({hx, -hy, -hz}, {-hx, -hy, -hz}, {-hx, hy, -hz}, {hx, hy, -hz}, {0, 0, -1});
        push_face({hx, -hy, -hz}, {hx, -hy, hz}, {hx, hy, hz}, {hx, hy, -hz}, {1, 0, 0});
        push_face({-hx, -hy, hz}, {-hx, -hy, -hz}, {-hx, hy, -hz}, {-hx, hy, hz}, {-1, 0, 0});

        auto vert_bytes = std::span<const u8>(reinterpret_cast<const u8 *>(verts.data()),
                                              verts.size() * sizeof(BasicVertex));
        add_mesh(renderer, vert_bytes, sizeof(BasicVertex), inds);
    }

    void SimpleRenderer::create_capsule_mesh(Renderer &renderer) {
        ArenaResource               resource(&m_arena);
        std::pmr::vector<BasicVertex> verts(&resource);
        std::pmr::vector<u32>         inds(&resource);
        constexpr glm::vec3      col{0.8f, 0.3f, 0.3f};

        constexpr u32 segs = CAPSULE_SEGMENTS;
        constexpr f32 r = CAPSULE_RADIUS;
        constexpr f32 hh = CAPSULE_HALF_HEIGHT;

        u32 bottom_ring = push_ring(verts, -hh, r, segs);
        u32 top_ring = push_ring(verts, hh, r, segs);

        for (u32 i = 0; i < segs; i++) {
            u32 ni = (i + 1) % segs;
            push_quad_indices(inds, bottom_ring + i, bottom_ring + ni, top_ring + ni,
                              top_ring + i);
        }

        u32 hemi_start = static_cast<u32>(verts.size());
        push_hemi_rings(verts, segs, -hh, r, -1.0f);
        verts.push_back({{0, -hh - r, 0}, col});
        u32 pole = static_cast<u32>(verts.size()) - 1;

        u32 prev = bottom_ring;
        for (u32 j = 0; j < CAPSULE_HEMI_SEGMENTS; j++) {
            u32 cur = hemi_start + j * segs;
            for (u32 i = 0; i < segs; i++) {
                u32 ni = (i + 1) % segs;
                push_quad_indices(inds, prev + i, prev + ni, cur + ni, cur + i);
            }
            prev = cur;
        }
        for (u32 i = 0; i < segs; i++) {
            u32 ni = (i + 1) % segs;
            inds.push_back(prev + i);
            inds.push_back(prev + ni);
            inds.push_back(pole);
        }

        hemi_start = static_cast<u32>(verts.size());
        push_hemi_rings(verts, segs, hh, r, 1.0f);
        verts.push_back({{0, hh + r, 0}, col});
        pole = static_cast<u32>(verts.size()) - 1;

        prev = top_ring;
        for (u32 j = 0; j < CAPSULE_HEMI_SEGMENTS; j++) {
            u32 cur = hemi_start + j * segs;
            for (u32 i = 0; i < segs; i++) {
                u32 ni = (i + 1) % segs;
                push_quad_indices(inds, prev + i, prev + ni, cur + ni, cur + i);
            }
            prev = cur;
        }
        for (u32 i = 0; i < segs; i++) {
            u32 ni = (i + 1) % segs;
            inds.push_back(prev + i);
            inds.push_back(pole);
            inds.push_back(prev + ni);
        }

        auto vert_bytes = std::span<const u8>(reinterpret_cast<const u8 *>(verts.data()),
                                              verts.size() * sizeof(BasicVertex));
        add_mesh(renderer, vert_bytes, sizeof(BasicVertex), inds);
    }

    void SimpleRenderer::add_passes(FrameGraph &graph, const Blackboard &blackboard,
                                     const Ecs &ecs) {
        MANTLE_CHECK(m_is_initialized);

        auto        backbuffer = blackboard.get<BbBackbuffer>().handle;
        const auto &camera_data = blackboard.get<BbCameraData>();
        const auto &fb_size = blackboard.get<BbFramebufferSize>();

        u32 width = fb_size.width;
        u32 height = fb_size.height;

        struct RenderPass {
            FGImageHandle   color;
            FGImageHandle   depth;
            FGBufferHandle  vertex_fg[2];
            FGBufferHandle  index_fg[2];
            u32             mesh_count;
            glm::mat4       view_proj;
        };

        graph.add_pass<RenderPass>(
            "Simple Render",
            [&](FrameGraphBuilder &builder, RenderPass &pass) {
                pass.depth = builder.create_image({
                    .width = width,
                    .height = height,
                    .format = ImageFormat::D32,
                    .usage = ImageUsage::Depth,
                });
                pass.color = builder.write(backbuffer, WriteUsage::ColorAttachment);
                builder.write(pass.depth, WriteUsage::DepthAttachment);
                pass.view_proj = camera_data.view_proj;
                pass.mesh_count = m_mesh_count;
                for (u32 i = 0; i < m_mesh_count; i++) {
                    pass.vertex_fg[i] = graph.import_buffer(m_meshes[i].vertex_buffer);
                    pass.index_fg[i] = graph.import_buffer(m_meshes[i].index_buffer);
                }
            },
            [this, width, height, &ecs](FGPassContext &ctx, const RenderPass &pass) {
                std::array<FGColorAttachment, 1> colors = {{
                    {
                        .image = pass.color,
                        .load = AttachmentLoad::Clear,
                        .store = AttachmentStore::Store,
                        .clear_r = 0.15f,
                        .clear_g = 0.15f,
                        .clear_b = 0.2f,
                        .clear_a = 1.0f,
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
                ctx.set_viewport(0, 0, static_cast<f32>(width), static_cast<f32>(height));
                ctx.set_scissor(0, 0, width, height);
                ctx.bind_pipeline(m_pipeline);

                ecs.foreach<const Transform, const MeshComponent>(
                    [&](const Transform &t, const MeshComponent &m) {
                        if (m.mesh_id >= pass.mesh_count) {
                            return;
                        }

                        glm::mat4 model = glm::translate(glm::mat4(1.0f), t.position);
                        model = glm::rotate(model, glm::radians(t.rotation.y),
                                            glm::vec3(0, 1, 0));
                        model = glm::scale(model, t.scale);

                        struct PushData {
                            glm::mat4 view_proj;
                            glm::mat4 model;
                        };
                        PushData pd = {pass.view_proj, model};

                        const auto &mesh = m_meshes[m.mesh_id];
                        ctx.push_constants(&pd, ShaderStage::Vertex);
                        ctx.bind_vertex_buffer(pass.vertex_fg[m.mesh_id], 0, 0);
                        ctx.bind_index_buffer(pass.index_fg[m.mesh_id], 0);
                        ctx.draw_indexed({mesh.index_count, 1, 0, 0, 0});
                    });

                ctx.end_rendering();
            });
    }

} // namespace mantle
