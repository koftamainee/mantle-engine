// Copyright (c) 2026 Mantle. All rights reserved.

#include "mantle/engine/engine.h"

#include <algorithm>
#include <cfloat>
#include <flecs.h>
#include <string_view>

#include "mantle/build_info/build_info.h"
#include "mantle/core/assert.h"
#include "mantle/core/logger.h"
#include "mantle/core/memory/memory_units.h"
#include "mantle/ecs/components.h"
#include "mantle/renderer/blackboard_types.h"
#include "mantle/system_info/system_info.h"
#include "mantle/window/window.h"

namespace mantle {
    Engine *Engine::s_instance = nullptr;

    Engine *Engine::instance() { return s_instance; }

    void Engine::init(const EngineConfig &cfg) {
        MANTLE_CHECK(!m_is_initialized);

        register_loggers();

        s_instance = this;
        m_logger = spdlog::get("engine").get();

        if constexpr (std::string_view(MANTLE_GIT_HASH).ends_with("-dirty")) {
            m_logger->warn("Working tree is dirty. Build may not be reproducible");
        }

        constexpr usize heap_total = megabytes(512);
        m_os_memory.init();
        m_heap.init(m_os_memory, heap_total);

        const MemoryBlock engine_block = m_heap.take(kilobytes(64));
        m_persistent_allocator.init(engine_block);
        m_heap_resource = TlsfResource(&m_persistent_allocator);

        const MemoryBlock window_block = m_heap.take(kilobytes(256));
        const MemoryBlock renderer_block = m_heap.take(megabytes(80));
        const MemoryBlock worker_pool_block = m_heap.take(megabytes(32));
        const MemoryBlock physics_block = m_heap.take(megabytes(100));

        m_window.init(cfg.window, window_block);

        m_window.set_resize_callback([this](u32 w, u32 h) {
            m_logger->info("Swapchain recreation triggered by window resize: {}x{}", w, h);
            m_renderer.resize_swapchain(w, h);
            // TODO: fix me with direct flecs world usage
            // m_world.foreach<Camera>(
            //     [w, h](Camera &cam) { cam.aspect = static_cast<f32>(w) / static_cast<f32>(h); });
        });

        m_worker_pool.init(8, megabytes(4), worker_pool_block);

        m_physics_system.init(physics_block);
        m_character.init(m_physics_system, {0.0f, 5.0f, 0.0f});

        m_renderer.init(m_window, false, renderer_block);

        m_input.init(&m_window);

        m_is_initialized = true;
        m_logger->info("Engine initialized. Starting the game");

        constexpr auto art = R"(
================================================================================

             /$$      /$$                       /$$     /$$
            | $$$    /$$$                      | $$    | $$
            | $$$$  /$$$$  /$$$$$$  /$$$$$$$  /$$$$$$  | $$  /$$$$$$
            | $$ $$/$$ $$ |____  $$| $$__  $$|_  $$_/  | $$ /$$__  $$
            | $$  $$$| $$  /$$$$$$$| $$  \ $$  | $$    | $$| $$$$$$$$
            | $$\  $ | $$ /$$__  $$| $$  | $$  | $$ /$$| $$| $$_____/
            | $$ \/  | $$|  $$$$$$$| $$  | $$  |  $$$$/| $$|  $$$$$$$
            |__/     |__/ \_______/|__/  |__/   \___/  |__/ \_______/

)";
        raw_logger()->info(art);
        raw_logger()->info("{}", build_string());
        raw_logger()->info("Built at: " MANTLE_BUILD_DATE " UTC");
        log_system_info(raw_logger(), m_renderer.gpu_name(), m_renderer.vram_bytes(),
                        m_window.get_width(), m_window.get_height(), false,
                        m_window.get_refresh_rate(), m_window.is_fullscreen());
        raw_logger()->info("================================================================"
                           "================");
        raw_logger()->info("");

        m_last_time = m_window.get_time_ns();
    }

    void Engine::run() {
        MANTLE_CHECK(m_is_initialized);

        m_fps_timer = static_cast<f32>(m_window.get_time_ns());
        while (!m_window.should_close()) {
            u64 current_time = m_window.get_time_ns();
            f32 delta_time = static_cast<f32>(current_time - m_last_time) / 1e9f;
            m_last_time = current_time;

            m_fps_frame_count++;
            m_fps_frametime_accum += delta_time;
            if (delta_time < m_fps_frametime_min) {
                m_fps_frametime_min = delta_time;
            }
            if (delta_time > m_fps_frametime_max) {
                m_fps_frametime_max = delta_time;
            }

            if (static_cast<f32>(current_time) - m_fps_timer >= 1e9f) {
                f32 avg_ms = m_fps_frametime_accum / static_cast<f32>(m_fps_frame_count) * 1000.0f;

                m_logger->info("{} FPS | {:>4.1f} ms | begin: {:.2f} | exec: {:.2f} | "
                               "end: {:.2f} | heap: {}/{} MB",
                               m_fps_frame_count, avg_ms,
                               m_fps_begin_accum / static_cast<f32>(m_fps_frame_count),
                               m_fps_exec_accum / static_cast<f32>(m_fps_frame_count),
                               m_fps_end_accum / static_cast<f32>(m_fps_frame_count),
                               m_heap.used() / 1024 / 1024, m_heap.reserved() / 1024 / 1024);

                m_fps_frame_count = 0;
                m_fps_frametime_accum = 0.0f;
                m_fps_frametime_min = FLT_MAX;
                m_fps_frametime_max = 0.0f;
                m_fps_begin_accum = 0.0f;
                m_fps_exec_accum = 0.0f;
                m_fps_end_accum = 0.0f;
                m_fps_timer = static_cast<f32>(current_time);
            }

            update(delta_time);
            render();
        }
    }

    void Engine::destroy() {
        if (m_is_initialized) {

            m_input.destroy();

            m_persistent_allocator.destroy();
            m_renderer.destroy();

            m_character.destroy();
            m_physics_system.destroy();

            m_worker_pool.destroy();

            m_window.destroy();

            m_is_initialized = false;
            s_instance = nullptr;
            m_logger->info("Engine destroyed");
            raw_logger()->info("see you soon~\n");
        }
    }

    flecs::world &Engine::world() {
        MANTLE_CHECK(m_is_initialized);
        return m_world;
    }

    InputSystem &Engine::input_system() {
        MANTLE_CHECK(m_is_initialized);
        return m_input;
    }

    void Engine::update(f32 dt) {
        m_window.update();
        m_input.update();
        (void)m_world.progress(dt);
        m_physics_system.update(dt);
    }

    void Engine::render() {
        u64              t0 = m_window.get_time_ns();
        Renderer::Result result = m_renderer.begin_frame();
        u64              t1 = m_window.get_time_ns();
        if (result == Renderer::Result::FrameNeedsResize) {
            auto [width, height] = m_window.get_framebuffer_size();
            m_logger->info("Swapchain recreation: after acquiring image ({}x{})", width, height);
            m_renderer.resize_swapchain(width, height);
            return;
        }

        FrameGraph    graph(&m_renderer.frame_arena());
        FGImageHandle backbuffer = graph.import_image(m_renderer.backbuffer());
        auto [width, height] = m_window.get_framebuffer_size();


        auto &bb = graph.blackboard();
        bb.add(BbBackbuffer {backbuffer});
        // bb.add(BbCameraData {view_proj});
        bb.add(BbFramebufferSize {width, height});


        m_renderer.execute(graph);
        u64 t2 = m_window.get_time_ns();

        result = m_renderer.end_frame();
        u64 t3 = m_window.get_time_ns();
        if (result == Renderer::Result::FrameNeedsResize) {
            Window::Properties::Size size = m_window.get_framebuffer_size();
            width = size.width;
            height = size.height;
            m_logger->info("Swapchain recreation: after presentation ({}x{})", width, height);
            m_renderer.resize_swapchain(width, height);
        }

        m_fps_begin_accum += static_cast<f32>(t1 - t0) / 1e6f;
        m_fps_exec_accum += static_cast<f32>(t2 - t1) / 1e6f;
        m_fps_end_accum += static_cast<f32>(t3 - t2) / 1e6f;
    }
} // namespace mantle
