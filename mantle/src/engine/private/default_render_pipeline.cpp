// Copyright 2026 Mantle
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "default_render_pipeline.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <cstdio>
#include <flecs.h>
#include <vector>

#include "mantle/core/assert.h"
#include "mantle/core/logger.h"
#include "mantle/ecs/components.h"
#include "mantle/renderer/blackboard_types.h"
#include "mantle/renderer/frame_graph.h"
#include "mantle/renderer/gpu_resource_manager.h"
#include "mantle/renderer/renderer.h"

namespace mantle {

    DefaultRenderPipeline::DefaultRenderPipeline(flecs::world &world, AssetManager &assets,
                                                 Renderer &renderer) :
        m_world(world),
        m_assets(assets),
        m_asset_base_path("assets"),
        m_logger(spdlog::get("engine").get()) {
        create_pipeline(renderer);
    }

    DefaultRenderPipeline::~DefaultRenderPipeline() {
        // Pipeline resources are destroyed by Renderer::destroy()
    }

    static std::vector<u32> read_spirv(const char *path) {
        FILE *f = fopen(path, "rb");
        if (!f) {
            spdlog::get("engine")->error("Failed to open shader: {}", path);
            return {};
        }
        fseek(f, 0, SEEK_END);
        long size = ftell(f);
        fseek(f, 0, SEEK_SET);
        std::vector<u32> code(static_cast<usize>(size) / sizeof(u32));
        if (fread(code.data(), 1, static_cast<usize>(size), f) != static_cast<usize>(size)) {
            fclose(f);
            spdlog::get("engine")->error("Failed to read shader: {}", path);
            return {};
        }
        fclose(f);
        return code;
    }

    void DefaultRenderPipeline::create_pipeline(Renderer &renderer) {
        auto &rm = renderer.resource_manager();
        auto  si = renderer.get_swapchain_info();

        std::string vert_path = m_asset_base_path + "/shaders/debug_mesh.spv";
        std::string frag_path = m_asset_base_path + "/shaders/debug_mesh.spv";

        auto vert_code = read_spirv(vert_path.c_str());
        auto frag_code = read_spirv(frag_path.c_str());

        if (vert_code.empty() || frag_code.empty()) {
            m_logger->error("DefaultRenderPipeline: failed to load shaders");
            return;
        }

        ShaderHandle vert_shader = rm.create_shader(vert_code);
        ShaderHandle frag_shader = rm.create_shader(frag_code);

        // Vertex input: position (float3) at binding 0, offset 0
        // The .mmesh vertex stride is 48 (POS 12 + NRM 12 + TAN 16 + UV 8)
        VertexBinding binding = {
            .binding = 0,
            .stride = 48,
            .per_instance = false,
        };
        VertexAttribute attr = {
            .location = 0,
            .binding = 0,
            .format = VertexFormat::Float3,
            .offset = 0,
        };

        PushConstantsRange pc_range = {
            .stage = ShaderStage::Vertex,
            .size = 128,
            .offset = 0,
        };

        ShaderModule shader_modules[] = {
            {.entry_point = "vert_main", .stage = ShaderStage::Vertex, .shader = vert_shader},
            {.entry_point = "frag_main", .stage = ShaderStage::Fragment, .shader = frag_shader},
        };

        VertexAttribute attributes[] = {attr};
        VertexBinding   bindings[] = {binding};

        ColorBlendAttachment blend_attachment = {
            .blend_enable = false,
            .color_write_mask = ColorWriteMask::RGBA,
        };
        ColorBlendAttachment blend_attachments[] = {blend_attachment};

        ImageFormat color_formats[] = {si.surface_format};

        GraphicsPipelineDesc desc = {
            .shaders = shader_modules,
            .vertex_input = {.bindings = bindings, .attributes = attributes},
            .input_assembly = {.topology = PrimitiveTopology::TriangleList},
            .rasterization =
                {
                    .polygon_mode = PolygonMode::Fill,
                    .cull_mode = CullMode::Back,
                    .front_face = FrontFace::CounterClockwise,
                },
            .multisample = {.rasterization_samples = SampleCount::x1},
            .depth_stencil =
                {
                    .depth_test_enable = true,
                    .depth_write_enable = true,
                    .depth_compare_op = CompareOp::Less,
                },
            .color_blend = {.attachments = blend_attachments},
            .color_formats = color_formats,
            .depth_format = ImageFormat::D32,
            .push_constants = {&pc_range, 1},
        };

        m_debug_pipeline = rm.create_graphics_pipeline(desc);
    }

    void DefaultRenderPipeline::add_passes(FrameGraph &graph) {
        if (!m_debug_pipeline.is_valid()) {
            return;
        }

        auto &bb = graph.blackboard();
        if (!bb.has<BbBackbuffer>() || !bb.has<BbFramebufferSize>() || !bb.has<BbCameraData>()) {
            return;
        }

        auto &backbuffer = bb.get<BbBackbuffer>();
        auto &fb_size = bb.get<BbFramebufferSize>();
        auto &camera = bb.get<BbCameraData>();

        ImageDesc depth_desc = {
            .width = fb_size.width,
            .height = fb_size.height,
            .format = ImageFormat::D32,
            .usage = ImageUsage::Depth,
        };

        // Pre-import all mesh vertex/index buffers into the frame graph
        u32 mesh_count = m_assets.mesh_count();
        struct MeshPair {
            FGBufferHandle vertex;
            FGBufferHandle index;
        };
        auto     &arena = graph.arena();
        MeshPair *mesh_buffers =
            static_cast<MeshPair *>(arena.push(mesh_count * sizeof(MeshPair), alignof(MeshPair)));

        for (u32 i = 0; i < mesh_count; i++) {
            auto &md = m_assets.mesh_data_by_index(i);
            if (md.vertex_buffer.is_valid()) {
                mesh_buffers[i].vertex = graph.import_buffer(md.vertex_buffer);
                mesh_buffers[i].index = graph.import_buffer(md.index_buffer);
            }
        }

        auto &debug_pipeline = m_debug_pipeline;

        using OpaqueData = struct {
            FGImageHandle depth;
        };

        const auto &asset_meshes = m_assets;

        graph.add_pass<OpaqueData>(
            "opaque",
            [depth_desc, &bb](FrameGraphBuilder &builder, OpaqueData &data) {
                data.depth = builder.create_image(depth_desc);
                builder.write(data.depth, WriteUsage::DepthAttachment);
                auto &backbuffer = bb.get<BbBackbuffer>();
                builder.write(backbuffer.handle, WriteUsage::ColorAttachment);
            },
            [&bb, mesh_count, mesh_buffers, &debug_pipeline, &asset_meshes,
             this](FGPassContext &ctx, const OpaqueData &data) {
                auto &backbuffer = bb.get<BbBackbuffer>();
                auto &fb_size = bb.get<BbFramebufferSize>();
                auto &camera = bb.get<BbCameraData>();

                FGColorAttachment colors[] = {{
                    .image = backbuffer.handle,
                    .load = AttachmentLoad::Clear,
                    .store = AttachmentStore::Store,
                    .clear_r = 0.1f,
                    .clear_g = 0.1f,
                    .clear_b = 0.2f,
                    .clear_a = 1.0f,
                }};

                FGDepthAttachment depth = {
                    .image = data.depth,
                    .load = AttachmentLoad::Clear,
                    .store = AttachmentStore::Store,
                    .clear_value = 1.0f,
                };

                FGRenderingInfo rendering_info = {
                    .colors = colors,
                    .depth = &depth,
                    .width = fb_size.width,
                    .height = fb_size.height,
                };

                ctx.begin_rendering(rendering_info);
                ctx.set_viewport(0, 0, static_cast<f32>(fb_size.width),
                                 static_cast<f32>(fb_size.height));
                ctx.set_scissor(0, 0, fb_size.width, fb_size.height);
                ctx.bind_pipeline(debug_pipeline);

                auto query = m_world.query<const MeshRenderer, const LocalTransform>();
                query.each([&](const MeshRenderer &mr, const LocalTransform &t) {
                    if (!mr.mesh.is_valid()) {
                        return;
                    }

                    u32 idx = mr.mesh.index;
                    if (idx >= mesh_count || !mesh_buffers[idx].vertex.is_valid()) {
                        return;
                    }

                    glm::mat4 model = glm::translate(glm::mat4(1.0f), t.translation);
                    model = model * glm::mat4_cast(t.rotation);
                    model = glm::scale(model, t.scale);

                    struct PCS {
                        glm::mat4 view_proj;
                        glm::mat4 model;
                    };
                    PCS pc = {camera.view_proj, model};
                    ctx.push_constants(&pc, ShaderStage::Vertex);

                    ctx.bind_vertex_buffer(mesh_buffers[idx].vertex, 0);
                    ctx.bind_index_buffer(mesh_buffers[idx].index);

                    auto &md = asset_meshes.mesh_data_by_index(idx);
                    for (auto &sm : md.submeshes) {
                        ctx.draw_indexed({
                            .index_count = sm.index_count,
                            .instance_count = 1,
                            .first_index = static_cast<u32>(sm.index_offset / sizeof(u32)),
                            .vertex_offset = static_cast<i32>(sm.vertex_offset),
                        });
                    }
                });

                ctx.end_rendering();
            });
    }

} // namespace mantle
