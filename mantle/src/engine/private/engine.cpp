#include "engine/engine.h"

#include "camera/camera.h"
#include "core/assert.h"
#include "core/memory/memory_units.h"
#include "helpers.h"
#include "renderer/renderer.h"
#include "renderer/utils.h"
#include "spdlog/spdlog.h"
#include "window/window.h"

namespace mantle {
    Engine::~Engine() { destroy(); }

    void Engine::init() {
        check(!m_is_initialized);

        m_os_memory.init();
        m_heap.init(m_os_memory, gigabytes(2));
        MemoryBlock memory = m_heap.take(megabytes(1));
        m_scratch_arena.init(memory);

        Window::Properties prop = {
            .title = "Mantle",
            .size = {.width = 2560, .height = 1600},
        };

        m_window.init(prop, &m_heap);

        m_renderer.init(m_window, false, &m_heap, &m_scratch_arena);

        m_camera.aspect = static_cast<f32>(prop.size.width) /
            static_cast<f32>(prop.size.height);

        m_window.set_resize_callback([this](u32 w, u32 h) {
            m_renderer.resize_swapchain(w, h);
            m_camera.aspect = static_cast<f32>(w) / static_cast<f32>(h);
        });

        m_last_time = 0;

        m_is_initialized = true;

        m_camera.position = glm::vec3(0.0f, 5.0f, 0.0f);

        ScopeArena arena(&m_scratch_arena);
        ArenaResource pmr(&m_scratch_arena);

        std::pmr::vector<u32> spv(&pmr);
        load_spv("assets/shaders/flat.spv", spv);
        ShaderHandle shader = m_renderer.resource_manager().create_shader(spv);

        ShaderModule shader_modules[] = {
            {"vert_main", ShaderStage::Vertex, shader},
            {"frag_main", ShaderStage::Fragment, shader},
        };

        ImageFormat color_format =
            m_renderer.get_swapchain_info().surface_format;

        ColorBlendState blend_state = {};
        ColorBlendAttachment attachment = {};
        blend_state.attachments = span(attachment);

        GraphicsPipelineDesc desc = {
            .shaders = shader_modules,
            .rasterization = {.cull_mode = CullMode::None},
            .color_blend = blend_state,
            .color_formats = span(color_format),
        };

        m_triangle_pipeline =
            m_renderer.resource_manager().create_graphics_pipeline(desc);

        m_renderer.resource_manager().destroy_shader(shader, true);

        {
            ScopeArena scope(&m_scratch_arena);
            ArenaResource resource(&m_scratch_arena);

            std::pmr::vector<u32> compute_spv(&resource);
            load_spv("assets/shaders/test_compute.spv", compute_spv);
            ShaderHandle compute_shader =
                m_renderer.resource_manager().create_shader(compute_spv);

            ShaderModule compute_module = {
                "compute_main", ShaderStage::Compute, compute_shader,
            };

            ComputePipelineDesc compute_desc = {
                .shader = compute_module,
                .push_constants = {ShaderStage::Compute, 0, 0},
            };

            m_test_compute_pipeline =
                m_renderer.resource_manager().create_compute_pipeline(compute_desc);

            m_renderer.resource_manager().destroy_shader(compute_shader, true);
        }

        m_rendering_arena.init(m_heap.take(megabytes(100)));

        spdlog::info("Engine is initialized. Starting the game");
    }

    void Engine::run() {
        while (!m_window.should_close()) {
            auto current_time = static_cast<f32>(m_window.get_time());
            f32 delta_time = current_time - m_last_time;
            m_last_time = current_time;

            update(delta_time);
            render();
        }
    }
    void Engine::destroy() {
        if (m_is_initialized) {
            m_renderer.destroy();
            m_window.destroy();
            m_is_initialized = false;
            spdlog::info("Engine is destroyed");
        }
    }

    void Engine::update(f32 delta_time) {
        m_window.update();

        auto [mouse_x, mouse_y] = m_window.get_mouse_position();

        float dx = m_mouse_x - mouse_x;
        float dy = m_mouse_y - mouse_y;
        m_mouse_x = mouse_x;
        m_mouse_y = mouse_y;
        dx *= m_mouse_sensitivity;
        dy *= m_mouse_sensitivity;

        m_camera.rotate(dx, dy);

        if (m_window.is_key_pressed(Window::Key::W)) {
            m_camera.position += m_camera.front * m_camera_speed * delta_time;
        }
        if (m_window.is_key_pressed(Window::Key::S)) {
            m_camera.position -= m_camera.front * m_camera_speed * delta_time;
        }
        if (m_window.is_key_pressed(Window::Key::A)) {
            m_camera.position -= m_camera.right * m_camera_speed * delta_time;
        }
        if (m_window.is_key_pressed(Window::Key::D)) {
            m_camera.position += m_camera.right * m_camera_speed * delta_time;
        }
        if (m_window.is_key_pressed(Window::Key::Shift)) {
            m_camera.position -= Camera::world_up * m_camera_speed * delta_time;
        }
        if (m_window.is_key_pressed(Window::Key::Space)) {
            m_camera.position += Camera::world_up * m_camera_speed * delta_time;
        }
        if (m_window.is_key_just_pressed(Window::Key::Ctrl)) {
            m_camera_speed = m_base_camera_speed * 2;
        }
        if (m_window.is_key_just_released(Window::Key::Ctrl)) {
            m_camera_speed = m_base_camera_speed;
        }
    }

    void Engine::render() {
        Renderer::Result result = m_renderer.begin_frame();
        if (result == Renderer::Result::FrameNeedsResize) {
            auto [width, height] = m_window.get_framebuffer_size();
            m_renderer.resize_swapchain(width, height);
            return;
        }

        RenderGraph graph(&m_rendering_arena);

        struct ComputePass final {
            RGBufferHandle out_storage;
        };

        struct TrianglePass final {
            RGImageHandle out_backbuffer;
        };

        RGImageHandle backbuffer =
            graph.import_image(m_renderer.backbuffer());
        auto [width, height] = m_window.get_framebuffer_size();

        graph.add_pass<ComputePass>(
            "Compute Test",
            [&](RenderGraphBuilder &builder, ComputePass &pass) {
                pass.out_storage =
                    builder.create_buffer({
                        .size = 64,
                        .usage = BufferUsage::Storage,
                        .memory = MemoryType::Gpu,
                    });
                builder.write(pass.out_storage, BufferWriteUsage::Storage);
            },
            [this](RenderPassContext &ctx, const ComputePass &) {
                ctx.bind_pipeline(m_test_compute_pipeline);
                ctx.dispatch({1, 1, 1});
            });

        graph.add_pass<TrianglePass>(
            "Triangle Pass",
            [&](RenderGraphBuilder &builder, TrianglePass &pass) {
                pass.out_backbuffer =
                    builder.write(backbuffer, WriteUsage::ColorAttachment);
            },
            [width, height, this](RenderPassContext &ctx,
                                  const TrianglePass &pass) {
                std::array<RGColorAttachment, 1> color_attachments = {{{
                    .image = pass.out_backbuffer,
                    .load = AttachmentLoad::Clear,
                    .store = AttachmentStore::Store,
                    .clear_r = 0.1f,
                    .clear_g = 0.2f,
                    .clear_b = 0.3f,
                    .clear_a = 1.0f,
                }}};
                ctx.begin_rendering({
                    .colors = color_attachments,
                    .width = width,
                    .height = height,
                });

                ctx.set_scissor(0, 0, width, height);
                ctx.set_viewport(0, 0, width, height);

                ctx.bind_pipeline(m_triangle_pipeline);
                ctx.draw({.vertex_count = 3});

                ctx.end_rendering();
            });

        m_renderer.execute(graph);

        result = m_renderer.end_frame();
        if (result == Renderer::Result::FrameNeedsResize) {
            Window::Properties::Size size = m_window.get_framebuffer_size();
            width = size.width;
            height = size.height;
            m_renderer.resize_swapchain(width, height);
        }
    }
} // namespace mantle
