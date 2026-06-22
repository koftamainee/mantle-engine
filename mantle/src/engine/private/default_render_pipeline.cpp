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
        m_renderer(renderer),
        m_asset_base_path("assets"),
        m_logger(spdlog::get("engine").get()) {
        set_ambient(glm::vec3(1.0f), 0.3f);
        add_directional_light(glm::vec3(0.5f, -1.0f, -0.5f), glm::vec3(1.0f), 1.5f);
        add_directional_light(glm::vec3(-0.3f, -0.5f, 0.7f), glm::vec3(0.5f, 0.6f, 0.8f), 0.5f);

        create_pipeline(renderer);
    }

    DefaultRenderPipeline::~DefaultRenderPipeline() {}

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

        std::string vert_path = m_asset_base_path + "/shaders/pbr_mesh.spv";
        std::string frag_path = m_asset_base_path + "/shaders/pbr_mesh.spv";

        auto vert_code = read_spirv(vert_path.c_str());
        auto frag_code = read_spirv(frag_path.c_str());

        if (vert_code.empty() || frag_code.empty()) {
            m_logger->error("DefaultRenderPipeline: failed to load PBR shaders");
            return;
        }

        ShaderHandle vert_shader = rm.create_shader(vert_code);
        ShaderHandle frag_shader = rm.create_shader(frag_code);

        VertexBinding binding = {
            .binding = 0,
            .stride = 48,
            .per_instance = false,
        };
        VertexAttribute attrs[] = {
            {.location = 0, .binding = 0, .format = VertexFormat::Float3, .offset = 0},
            {.location = 1, .binding = 0, .format = VertexFormat::Float3, .offset = 12},
            {.location = 2, .binding = 0, .format = VertexFormat::Float4, .offset = 24},
            {.location = 3, .binding = 0, .format = VertexFormat::Float2, .offset = 40},
        };

        PushConstantsRange pc_ranges[] = {
            {.stage = ShaderStage::VertexFragment, .size = 84, .offset = 0},
        };

        ShaderModule shader_modules[] = {
            {.entry_point = "vert_main", .stage = ShaderStage::Vertex, .shader = vert_shader},
            {.entry_point = "frag_main", .stage = ShaderStage::Fragment, .shader = frag_shader},
        };

        VertexBinding                    bindings[] = {binding};
        std::span<const VertexAttribute> attributes_span(attrs);

        ColorBlendAttachment blend_attachment = {
            .blend_enable = false,
            .color_write_mask = ColorWriteMask::RGBA,
        };
        ColorBlendAttachment blend_attachments[] = {blend_attachment};

        ImageFormat color_formats[] = {si.surface_format};

        GraphicsPipelineDesc desc = {
            .shaders = shader_modules,
            .vertex_input = {.bindings = bindings, .attributes = attributes_span},
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
            .push_constants = pc_ranges,
        };

        m_pbr_pipeline = rm.create_graphics_pipeline(desc);

        BufferDesc frame_desc = {
            .size = 256 * 64,
            .usage = BufferUsage::Storage,
            .memory = MemoryType::CpuToGpu,
        };
        m_frame_data_buffer = rm.create_buffer(frame_desc, true);
        m_frame_data_buffer_idx = rm.get_bindless_index(m_frame_data_buffer);

        BufferDesc light_desc = {
            .size = sizeof(GPULightData),
            .usage = BufferUsage::Storage,
            .memory = MemoryType::CpuToGpu,
        };
        m_light_buffer = rm.create_buffer(light_desc, true);
        m_light_buffer_idx = rm.get_bindless_index(m_light_buffer);
    }

    void DefaultRenderPipeline::rebuild_materials() {
        if (m_material_buffer.is_valid()) {
            return;
        }
        auto &rm = m_renderer.resource_manager();
        m_material_buffer = m_assets.build_material_buffer();
        if (m_material_buffer.is_valid()) {
            m_material_buffer_idx = rm.get_bindless_index(m_material_buffer);
        } else {
            m_logger->warn("No materials loaded, skipping material buffer");
            return;
        }
        m_assets.collect_pending_uploads(m_pending_uploads);
    }

    void DefaultRenderPipeline::add_passes(FrameGraph &graph) {
        if (!m_pbr_pipeline.is_valid()) {
            return;
        }

        if (!m_material_buffer.is_valid() && m_assets.mesh_count() > 0) {
            rebuild_materials();
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

        if (!m_pending_uploads.empty()) {
            auto uploads = std::move(m_pending_uploads);
            m_pending_uploads.clear();

            struct UploadData {
                std::vector<FGBufferHandle> srcs;
                std::vector<FGImageHandle>  dsts;
                std::vector<u32>            mips;
                std::vector<u32>            offsets;
            };

            u32 total_mips = 0;
            for (auto &u : uploads) {
                total_mips += u.mip_count;
            }

            graph.add_pass<UploadData>(
                "texture_upload",
                [&graph, uploads, total_mips](FrameGraphBuilder &builder, UploadData &data) {
                    data.srcs.reserve(total_mips);
                    data.dsts.reserve(total_mips);
                    data.mips.reserve(total_mips);
                    data.offsets.reserve(total_mips);

                    for (auto &u : uploads) {
                        auto src = graph.import_buffer(u.staging);
                        auto dst = graph.import_image(u.image);
                        builder.write(dst, WriteUsage::TransferDst);
                        for (u32 mi = 0; mi < u.mip_count; mi++) {
                            data.srcs.push_back(src);
                            data.dsts.push_back(dst);
                            data.mips.push_back(mi);
                            data.offsets.push_back(u.mip_offsets[mi]);
                        }
                    }
                },
                [](FGPassContext &ctx, const UploadData &data) {
                    for (usize i = 0; i < data.srcs.size(); i++) {
                        ctx.copy_buffer_to_image({
                            .src = data.srcs[i],
                            .dst = data.dsts[i],
                            .buffer_offset = data.offsets[i],
                            .mip_level = data.mips[i],
                        });
                    }
                });
        }

        auto &pbr_pipeline = m_pbr_pipeline;
        auto &material_buffer_idx = m_material_buffer_idx;
        auto &frame_data_buffer_idx = m_frame_data_buffer_idx;
        auto &frame_data_buffer = m_frame_data_buffer;
        auto &light_buffer_idx = m_light_buffer_idx;
        auto &light_buffer = m_light_buffer;
        auto &light_data = m_light_data;

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
            [&bb, mesh_count, mesh_buffers, &pbr_pipeline, &asset_meshes, this,
             &material_buffer_idx, &frame_data_buffer_idx, &frame_data_buffer, &light_buffer_idx,
             &light_buffer, &light_data](FGPassContext &ctx, const OpaqueData &data) {
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
                ctx.bind_pipeline(pbr_pipeline);

                struct EntityDraw {
                    u32 entity_index;
                    u32 mesh_index;
                    u32 material_offset;
                };
                std::vector<EntityDraw> draw_list;
                draw_list.reserve(256);

                auto &rm = m_renderer.resource_manager();
                auto  query = m_world.query<const MeshRenderer, const LocalTransform>();
                query.each([&](const MeshRenderer &mr, const LocalTransform &t) {
                    if (!mr.mesh.is_valid()) {
                        return;
                    }
                    u32 idx = mr.mesh.index;
                    if (idx >= mesh_count || !mesh_buffers[idx].vertex.is_valid()) {
                        return;
                    }
                    draw_list.push_back({
                        .entity_index = static_cast<u32>(draw_list.size()),
                        .mesh_index = idx,
                        .material_offset = 0,
                    });
                });

                {
                    struct ModelMatrix {
                        glm::mat4 m;
                    };
                    std::vector<ModelMatrix> models(draw_list.size());
                    u32                      di = 0;
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
                        models[di++].m = model;
                    });
                    rm.update_buffer(frame_data_buffer, models.data(),
                                     draw_list.size() * sizeof(ModelMatrix));
                }

                light_data.camera_pos = glm::vec4(camera.camera_pos, 1.0f);
                rm.update_buffer(light_buffer, &light_data, sizeof(GPULightData));
                struct PCPacked {
                    glm::mat4 view_proj;
                    u32       entity_index;
                    u32       material_index;
                    u32       material_buf;
                    u32       frame_data_buf;
                    u32       light_buf;
                };

                for (auto &draw : draw_list) {
                    u32 idx = draw.mesh_index;

                    ctx.bind_vertex_buffer(mesh_buffers[idx].vertex, 0);
                    ctx.bind_index_buffer(mesh_buffers[idx].index);

                    auto &md = asset_meshes.mesh_data_by_index(idx);
                    u32   lod_end = md.lod_count > 1 ? md.lod_first_submesh[1]
                                                     : static_cast<u32>(md.submeshes.size());
                    for (u32 si = 0; si < lod_end; si++) {
                        auto &sm = md.submeshes[si];

                        PCPacked pc = {
                            .view_proj = camera.view_proj,
                            .entity_index = draw.entity_index,
                            .material_index = si,
                            .material_buf = material_buffer_idx,
                            .frame_data_buf = frame_data_buffer_idx,
                            .light_buf = light_buffer_idx,
                        };
                        ctx.push_constants(&pc, ShaderStage::VertexFragment);

                        ctx.draw_indexed({
                            .index_count = sm.index_count,
                            .instance_count = 1,
                            .first_index = static_cast<u32>(sm.index_offset / sizeof(u32)),
                            .vertex_offset = 0,
                        });
                    }
                }

                ctx.end_rendering();
            });
    }

    void DefaultRenderPipeline::set_ambient(glm::vec3 color, f32 intensity) {
        m_light_data.ambient = glm::vec4(color, intensity);
    }

    u32 DefaultRenderPipeline::add_directional_light(glm::vec3 direction, glm::vec3 color,
                                                     f32 intensity) {
        u32 idx = m_light_data.light_count;
        if (idx >= MAX_DIRECTIONAL_LIGHTS) {
            m_logger->warn("Max directional lights reached ({})", MAX_DIRECTIONAL_LIGHTS);
            return UINT32_MAX;
        }
        m_light_data.lights[idx] = {
            .direction = glm::vec4(glm::normalize(direction), intensity),
            .color = glm::vec4(color, 1.0f),
        };
        m_light_data.light_count = idx + 1;
        return idx;
    }

    void DefaultRenderPipeline::clear_lights() { m_light_data.light_count = 0; }

} // namespace mantle
