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

        {
            ScopeArena scope(&m_scratch_arena);
            ArenaResource resource(&m_scratch_arena);

            std::pmr::vector<u32> spv(&resource);
            load_spv("assets/shaders/dda.spv", spv);
            ShaderHandle shader =
                m_renderer.resource_manager().create_shader(spv);

            m_dda_pipeline =
                m_renderer.resource_manager().create_compute_pipeline({
                    .shader = {"main", ShaderStage::Compute, shader},
                    .push_constants = {ShaderStage::Compute, sizeof(u32), 0},
                });
            m_renderer.resource_manager().destroy_shader(shader, true);
        }

        {
            ScopeArena scope(&m_scratch_arena);
            ArenaResource resource(&m_scratch_arena);

            std::pmr::vector<u32> spv(&resource);
            load_spv("assets/shaders/blit.spv", spv);
            ShaderHandle shader =
                m_renderer.resource_manager().create_shader(spv);

            ShaderModule modules[] = {
                {"vert_main", ShaderStage::Vertex, shader},
                {"frag_main", ShaderStage::Fragment, shader},
            };
            ImageFormat color_format =
                m_renderer.get_swapchain_info().surface_format;
            ColorBlendAttachment attachment = {};
            ColorBlendState blend = {};
            blend.attachments = span(attachment);
            PushConstantsRange blit_pc[] = {
                {ShaderStage::Fragment, sizeof(u32) * 2, 0},
            };

            m_blit_pipeline =
                m_renderer.resource_manager().create_graphics_pipeline({
                    .shaders = modules,
                    .rasterization = {.cull_mode = CullMode::None},
                    .color_blend = blend,
                    .color_formats = span(color_format),
                    .push_constants = blit_pc,
                });
            m_renderer.resource_manager().destroy_shader(shader, true);
        }

        m_blit_sampler = m_renderer.resource_manager().create_sampler({
            .min_filter = Filter::Nearest,
            .mag_filter = Filter::Nearest,
        });
        m_blit_sampler_index =
            m_renderer.resource_manager().get_bindless_index(m_blit_sampler);

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


        RGImageHandle backbuffer = graph.import_image(m_renderer.backbuffer());
        auto [width, height] = m_window.get_framebuffer_size();

        struct DDAPass final {
            RGImageHandle out_image;
        };

        struct BlitPass final {
            RGImageHandle in_image;
            RGImageHandle out_backbuffer;
        };

        const auto &dda = graph.add_pass<DDAPass>(
            "DDA Raycast",
            [&](RenderGraphBuilder &builder, DDAPass &pass) {
                pass.out_image = builder.create_image({
                    .width = width,
                    .height = height,
                    .format = ImageFormat::Rgba8,
                    .usage = ImageUsage::Storage | ImageUsage::Sampled,
                });
                builder.write(pass.out_image, WriteUsage::StorageWrite);
            },
            [width, height, this](RenderPassContext &ctx, const DDAPass &pass) {
                ctx.bind_pipeline(m_dda_pipeline);
                u32 storage_index = ctx.get_bindless_index(
                    pass.out_image, BindlessImageType::Storage);
                ctx.push_constants(&storage_index, ShaderStage::Compute);
                ctx.dispatch({(width + 7) / 8, (height + 7) / 8, 1});
            });

        graph.add_pass<BlitPass>(
            "Blit",
            [&](RenderGraphBuilder &builder, BlitPass &pass) {
                pass.in_image = builder.read(dda.out_image, ReadUsage::Sampled);
                pass.out_backbuffer =
                    builder.write(backbuffer, WriteUsage::ColorAttachment);
            },
            [width, height, this](RenderPassContext &ctx,
                                  const BlitPass &pass) {
                std::array<RGColorAttachment, 1> colors = {{{
                    .image = pass.out_backbuffer,
                    .load = AttachmentLoad::Clear,
                    .store = AttachmentStore::Store,
                }}};
                ctx.begin_rendering(
                    {.colors = colors, .width = width, .height = height});
                ctx.set_viewport(0, 0, width, height);
                ctx.set_scissor(0, 0, width, height);
                ctx.bind_pipeline(m_blit_pipeline);

                struct BlitPC {
                    u32 sampled_index;
                    u32 sampler_index;
                };
                BlitPC pc = {
                    ctx.get_bindless_index(pass.in_image,
                                           BindlessImageType::Sampled),
                    m_blit_sampler_index,
                };
                ctx.push_constants(&pc, ShaderStage::Fragment);

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
