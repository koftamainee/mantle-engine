#include "engine/engine.h"

#include <algorithm>
#include <cfloat>
#include <random>
#include <string_view>

#include "build_info/build_info.h"
#include "core/assert.h"
#include "core/logger.h"
#include "core/memory/memory_units.h"
#include "renderer/blackboard_types.h"
#include "system_info/system_info.h"
#include "window/window.h"
#include "world/light_propagation.h"

namespace mantle {
    namespace {
        void get_neighbors(ChunkStorageSystem &storage, glm::ivec3 pos,
                           Chunk *out[6]) {
            static const glm::ivec3 offsets[6] = {
                {-1, 0, 0}, {1, 0, 0},  {0, -1, 0},
                {0, 1, 0},  {0, 0, -1}, {0, 0, 1},
            };
            for (u32 i = 0; i < 6; i++) {
                glm::ivec3 nb_pos = pos + offsets[i];
                out[i] = storage.has_chunk(nb_pos) ? &storage.get_chunk(nb_pos)
                                                   : nullptr;
            }
        }

        struct GenTask {
            const ChunkGenerationSystem *gen;
            Chunk *chunk;
            glm::ivec3 pos;
        };

        void gen_work(ArenaAllocator &, void *data) {
            auto *d = static_cast<GenTask *>(data);
            d->gen->generate(*d->chunk, d->pos);
        }

    } // namespace

    Engine::~Engine() { destroy(); }

    void Engine::init() {
        MANTLE_CHECK(!m_is_initialized);

        m_logger = spdlog::get("engine").get();

        if constexpr (std::string_view(MANTLE_GIT_HASH).ends_with("-dirty")) {
            m_logger->warn(
                "Working tree is dirty. Build may not be reproducible");
        }

        m_os_memory.init();
        m_heap.init(m_os_memory, gigabytes(2));
        MemoryBlock memory = m_heap.take(megabytes(1));
        m_scratch_arena.init(memory);

        m_window.init({}, &m_heap);

        f32 camera_aspect = static_cast<f32>(m_window.get_width())
           / static_cast<f32>(m_window.get_height());

        m_ecs.init(m_window, camera_aspect);

        m_renderer.init(m_window, false, &m_heap, &m_scratch_arena);

        m_window.set_resize_callback([this](u32 w, u32 h) {
            m_logger->info(
                "Swapchain recreation triggered by window resize: {}x{}", w, h);
            m_renderer.resize_swapchain(w, h);
            m_ecs.set_camera_aspect(static_cast<f32>(w) / static_cast<f32>(h));
        });


        m_last_time = 0;

        constexpr u32 max_chunks = 150;
        constexpr i32 R = 2;
        constexpr u32 num_chunks = (R * 2 + 1) * (R * 2 + 1) * (R * 2 + 1);

        m_chunk_generation_system.init(std::random_device{}() % 1000);

        m_worker_pool.init(4, megabytes(8), &m_heap);

        m_meshing_arena.init(m_heap.take(megabytes(128)));
        m_chunk_rendering_system.init(m_renderer, m_scratch_arena, max_chunks);
        m_chunk_meshing_system.init();
        m_chunk_storage_system.init(max_chunks, &m_heap);

        m_rendering_arena.init(m_heap.take(megabytes(100)));

        m_logger->info("Generating chunks. This might take a while...");

        u64 gen_start = m_window.get_time_ns();
        {
            std::vector<GenTask> tasks;
            tasks.reserve(num_chunks);
            for (i32 x = -R; x <= R; x++) {
                for (i32 y = -R; y <= R; y++) {
                    for (i32 z = -R; z <= R; z++) {
                        glm::ivec3 pos(x, y, z);
                        u32 idx = m_chunk_storage_system.add_chunk(pos);
                        tasks.push_back({&m_chunk_generation_system,
                                         &m_chunk_storage_system.get_chunk(idx),
                                         pos});
                        m_worker_pool.submit(gen_work, &tasks.back());
                    }
                }
            }
            m_worker_pool.wait();
        }
        f32 gen_elapsed =
            static_cast<f32>(m_window.get_time_ns() - gen_start) / 1e6f;
        m_logger->info("Generation: {} chunks, {:.2f} ms total, {:.4f} ms avg",
                       num_chunks, gen_elapsed, gen_elapsed / num_chunks);

        u64 light_start = m_window.get_time_ns();
        {
            for (i32 x = -R; x <= R; x++) {
                for (i32 y = -R; y <= R; y++) {
                    for (i32 z = -R; z <= R; z++) {
                        glm::ivec3 pos(x, y, z);
                        u32 idx = m_chunk_storage_system.get_index(pos);
                        init_chunk_light(m_chunk_storage_system.get_chunk(idx));
                    }
                }
            }

            for (u8 level = 8; level > 0; level--) {
                for (i32 x = -R; x <= R; x++) {
                    for (i32 y = -R; y <= R; y++) {
                        for (i32 z = -R; z <= R; z++) {
                            glm::ivec3 pos(x, y, z);
                            u32 idx = m_chunk_storage_system.get_index(pos);
                            Chunk *neighbors[6];
                            get_neighbors(m_chunk_storage_system, pos,
                                          neighbors);
                            propagate_chunk_light_level(
                                m_chunk_storage_system.get_chunk(idx),
                                neighbors, level);
                        }
                    }
                }
            }
        }
        f32 light_elapsed =
            static_cast<f32>(m_window.get_time_ns() - light_start) / 1e6f;
        m_logger->info("Light propagation: {:.2f} ms", light_elapsed);

        u64 mesh_start = m_window.get_time_ns();
        m_chunk_meshing_system.upload_dirty(m_renderer, m_chunk_storage_system,
                                            m_meshing_arena, &m_worker_pool,
                                            m_chunk_rendering_system);
        f32 mesh_elapsed =
            static_cast<f32>(m_window.get_time_ns() - mesh_start) / 1e6f;
        m_logger->info("Meshing: {} chunks, {:.2f} ms total, {:.4f} ms avg",
                       num_chunks, mesh_elapsed, mesh_elapsed / num_chunks);

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
        log_system_info(raw_logger(), m_renderer.gpu_name(),
                        m_renderer.vram_bytes(), m_window.get_width(),
                        m_window.get_height(), false,
                        m_window.get_refresh_rate(), m_window.is_fullscreen());
        raw_logger()->info(
            "================================================================"
            "================");
        raw_logger()->info("");
    }

    void Engine::run() {
        m_fps_timer = static_cast<f32>(m_window.get_time_ns());
        while (!m_window.should_close()) {
            auto current_time = static_cast<f32>(m_window.get_time_ns());
            f32 delta_time = (current_time - m_last_time) / 1e9f;
            m_last_time = current_time;

            m_fps_frame_count++;
            m_fps_frametime_accum += delta_time;
            if (delta_time < m_fps_frametime_min)
                m_fps_frametime_min = delta_time;
            if (delta_time > m_fps_frametime_max)
                m_fps_frametime_max = delta_time;

            if (current_time - m_fps_timer >= 1e9f) {
                f32 avg_ms = m_fps_frametime_accum /
                    static_cast<f32>(m_fps_frame_count) * 1000.0f;

                m_logger->info(
                    "{} FPS | {:>4.1f} ms | begin: {:.2f} | exec: {:.2f} | "
                    "end: {:.2f} | heap: {}/{} MB",
                    m_fps_frame_count, avg_ms,
                    m_fps_begin_accum / static_cast<f32>(m_fps_frame_count),
                    m_fps_exec_accum / static_cast<f32>(m_fps_frame_count),
                    m_fps_end_accum / static_cast<f32>(m_fps_frame_count),
                    m_heap.used() / 1024 / 1024,
                    m_heap.reserved() / 1024 / 1024);

                m_fps_frame_count = 0;
                m_fps_frametime_accum = 0.0f;
                m_fps_frametime_min = FLT_MAX;
                m_fps_frametime_max = 0.0f;
                m_fps_begin_accum = 0.0f;
                m_fps_exec_accum = 0.0f;
                m_fps_end_accum = 0.0f;
                m_fps_timer = current_time;
            }

            update(delta_time);
            render();
        }
    }
    void Engine::destroy() {
        if (m_is_initialized) {
            m_ecs.destroy();
            m_worker_pool.destroy();
            m_chunk_storage_system.destroy();
            m_chunk_generation_system.destroy();
            m_chunk_meshing_system.destroy();
            m_chunk_rendering_system.destroy();
            m_renderer.destroy();
            m_window.destroy();
            m_is_initialized = false;
            m_logger->info("Engine destroyed");
        }
    }

    void Engine::update(f32 delta_time) {
        m_window.update();
        m_ecs.update(delta_time);
    }

    void Engine::render() {
        u64 t0 = m_window.get_time_ns();
        Renderer::Result result = m_renderer.begin_frame();
        u64 t1 = m_window.get_time_ns();
        if (result == Renderer::Result::FrameNeedsResize) {
            auto [width, height] = m_window.get_framebuffer_size();
            m_logger->info(
                "Swapchain recreation: after acquiring image ({}x{})", width,
                height);
            m_renderer.resize_swapchain(width, height);
            return;
        }

        m_meshing_arena.reset();
        m_chunk_meshing_system.upload_dirty(m_renderer, m_chunk_storage_system,
                                            m_meshing_arena, &m_worker_pool,
                                            m_chunk_rendering_system);

        FrameGraph graph(&m_rendering_arena);
        FGImageHandle backbuffer = graph.import_image(m_renderer.backbuffer());
        auto [width, height] = m_window.get_framebuffer_size();

        auto &bb = graph.blackboard();
        bb.add(BbBackbuffer{backbuffer});
        bb.add(BbCameraData{m_ecs.camera_view_proj()});
        bb.add(BbFramebufferSize{width, height});

        m_chunk_rendering_system.add_passes(graph, bb);

        m_renderer.execute(graph);
        u64 t2 = m_window.get_time_ns();

        result = m_renderer.end_frame();
        u64 t3 = m_window.get_time_ns();
        if (result == Renderer::Result::FrameNeedsResize) {
            Window::Properties::Size size = m_window.get_framebuffer_size();
            width = size.width;
            height = size.height;
            m_logger->info("Swapchain recreation: after presentation ({}x{})",
                           width, height);
            m_renderer.resize_swapchain(width, height);
        }

        m_fps_begin_accum += static_cast<f32>(t1 - t0) / 1e6f;
        m_fps_exec_accum += static_cast<f32>(t2 - t1) / 1e6f;
        m_fps_end_accum += static_cast<f32>(t3 - t2) / 1e6f;
    }
} // namespace mantle
