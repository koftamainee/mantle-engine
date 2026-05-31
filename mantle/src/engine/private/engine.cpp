#include "engine/engine.h"

#include <algorithm>
#include <cfloat>
#include <random>

#include "core/assert.h"
#include "core/memory/memory_units.h"
#include "renderer/blackboard_types.h"
#include "spdlog/spdlog.h"
#include "window/window.h"

namespace mantle {
    namespace {
        struct GenTask {
            const ChunkGenerationSystem *gen;
            Chunk *chunk;
            glm::ivec3 pos;
        };

        void gen_work(ArenaAllocator &, void *data) {
            auto *d = static_cast<GenTask *>(data);
            d->gen->generate(*d->chunk, d->pos);
        }

    } // anonymous namespace

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

        m_camera.position = glm::vec3(1.6f, 3.0f, 1.6f);

        m_last_time = 0;

        constexpr u32 max_chunks = 150;
        constexpr i32 R = 2;
        constexpr u32 num_chunks = (R * 2 + 1) * (R * 2 + 1) * (R * 2 + 1);

        m_chunk_generation_system.init(std::random_device{}() % 1000);

        m_worker_pool.init(4, megabytes(8), &m_heap);

        m_meshing_arena.init(m_heap.take(megabytes(128)));
        m_chunk_meshing_system.init(m_renderer, m_scratch_arena, max_chunks);
        m_chunk_storage_system.init(max_chunks, &m_heap);

        m_rendering_arena.init(m_heap.take(megabytes(100)));

        spdlog::info("Generating chunks. This might take a while...");

        f32 gen_start = m_window.get_time();
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
        f32 gen_elapsed = (m_window.get_time() - gen_start) * 1000.0f;
        spdlog::info("Generation: {} chunks, {:.2f} ms total, {:.4f} ms avg",
                     num_chunks, gen_elapsed, gen_elapsed / num_chunks);

        f32 mesh_start = m_window.get_time();
        m_chunk_meshing_system.upload_dirty(m_renderer, m_chunk_storage_system,
                                            m_meshing_arena, &m_worker_pool);
        f32 mesh_elapsed = (m_window.get_time() - mesh_start) * 1000.0f;
        spdlog::info("Meshing: {} chunks, {:.2f} ms total, {:.4f} ms avg",
                     num_chunks, mesh_elapsed, mesh_elapsed / num_chunks);

        m_is_initialized = true;
        spdlog::info("Engine is initialized. Starting the game");
    }

    void Engine::run() {
        m_fps_timer = static_cast<f32>(m_window.get_time());
        while (!m_window.should_close()) {
            auto current_time = static_cast<f32>(m_window.get_time());
            f32 delta_time = current_time - m_last_time;
            m_last_time = current_time;

            m_fps_frame_count++;
            m_fps_frametime_accum += delta_time;
            if (delta_time < m_fps_frametime_min)
                m_fps_frametime_min = delta_time;
            if (delta_time > m_fps_frametime_max)
                m_fps_frametime_max = delta_time;

            if (current_time - m_fps_timer >= 1.0f) {
                f32 avg_ms = m_fps_frametime_accum /
                    static_cast<f32>(m_fps_frame_count) * 1000.0f;

                spdlog::info("{} FPS | {:>4.1f} ms", m_fps_frame_count, avg_ms);

                m_fps_frame_count = 0;
                m_fps_frametime_accum = 0.0f;
                m_fps_frametime_min = FLT_MAX;
                m_fps_frametime_max = 0.0f;
                m_fps_timer = current_time;
            }

            update(delta_time);
            render();
        }
    }
    void Engine::destroy() {
        if (m_is_initialized) {
            m_worker_pool.destroy();
            m_chunk_storage_system.destroy();
            m_chunk_meshing_system.destroy();
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

        m_meshing_arena.reset();
        m_chunk_meshing_system.upload_dirty(m_renderer, m_chunk_storage_system,
                                            m_meshing_arena, &m_worker_pool);

        FrameGraph graph(&m_rendering_arena);
        FGImageHandle backbuffer = graph.import_image(m_renderer.backbuffer());
        auto [width, height] = m_window.get_framebuffer_size();

        auto &bb = graph.blackboard();
        bb.add(BbBackbuffer{backbuffer});
        bb.add(BbCameraData{m_camera.gpu_data().view_proj});
        bb.add(BbFramebufferSize{width, height});

        m_chunk_meshing_system.add_passes(graph, bb);

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
