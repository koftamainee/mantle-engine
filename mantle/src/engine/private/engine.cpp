#include "engine/engine.h"

#include <algorithm>
#include <ranges>

#include "camera/camera.h"
#include "core/assert.h"
#include "glm/gtx/transform.hpp"
#include "mesh/mesh.h"
#include "renderer/renderer.h"
#include "spdlog/spdlog.h"
#include "window/window.h"
#include "world/chunk_generation_system.h"
#include "world/chunk_meshing_system.h"

namespace mantle {
    void Engine::init() {
        check(!m_is_initialized);

        m_os_memory.init();
        m_heap.init(m_os_memory, 2u * 1024 * 1024 * 1024); // 2 GB
        MemoryBlock memory = m_heap.take(1u * 1024 * 1024); // 1 MB
        m_scratch_arena.init(memory);

        Window::Properties prop = {
            .title = "Mantle",
            .size = {.width = 2560, .height = 1600},
        };
        // there is nothing we can do about allocations inside glfw
        m_window.init(prop);

        m_renderer.init(m_window, &m_heap, &m_scratch_arena);

        m_camera.aspect = static_cast<f32>(prop.size.width) /
            static_cast<f32>(prop.size.height);

        m_window.set_resize_callback([&](u32 w, u32 h) {
            m_renderer.resize(w, h);
            m_camera.aspect = static_cast<f32>(w) / static_cast<f32>(h);
        });

        GPUResourceManager &resources = m_renderer.get_resource_manager();

        auto chunk_mesher = [&](Chunk &chunk) {
            chunk.is_dirty = false;
            Mesh mesh = ChunkMeshingSystem::build(chunk.data);
            if (mesh.vertices.empty()) {
                return;
            }

            glm::vec3 world_pos = glm::vec3(chunk.position) *
                static_cast<f32>(Chunk::Data::chunk_size);

            ChunkRenderData render_data = {
                .mesh = resources.upload_mesh(mesh.vertices, mesh.indices),
                .model = glm::translate(glm::mat4{1.0f}, world_pos),
                .aabb = {world_pos,
                         world_pos + static_cast<f32>(Chunk::Data::chunk_size)},
            };
            m_chunk_render_data.insert({chunk.position, render_data});
        };


        // TODO: carry out this logic to chunk streamer system
        for (i32 x = -5; x < 5; x++) {
            for (i32 y = -1; y < 5; y++) {
                for (i32 z = -5; z < 5; z++) {
                    u32 index = m_chunk_storage.chunks.size();
                    m_chunk_storage.chunks.push_back({});
                    m_chunk_storage.index.insert({{x, y, z}, index});

                    m_chunk_storage.chunks[index].position = {x, y, z};
                    ChunkGenerationSystem::generate(
                        m_chunk_storage.chunks[index].data, {x, y, z});
                    chunk_mesher(m_chunk_storage.chunks[index]);
                }
            }
        }


        m_last_time = 0;

        m_is_initialized = true;

        m_camera.position = glm::vec3(0.0f, 5.0f, 0.0f);
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
        if (m_window.is_key_pressed(Window::Key::Ctrl)) {
            m_camera_speed = m_base_camera_speed * 2;
        } else {
            m_camera_speed = m_base_camera_speed;
        }

        constexpr i32 max_chunks_per_frame = 10;

        i32 processed = 0;
        while (!m_dirty_chunks.empty() && processed < max_chunks_per_frame) {
            glm::ivec3 pos = m_dirty_chunks.front();
            m_dirty_chunks.pop();

            Chunk &chunk = m_chunk_storage.chunks[index(pos.x, pos.y, pos.z)];
            if (!chunk.is_dirty) {
                continue;
            }

            chunk.is_dirty = false;

            Mesh mesh = ChunkMeshingSystem::build(chunk.data);
            if (mesh.vertices.empty()) {
                continue;
            }

            glm::vec3 world_pos =
                glm::vec3(pos) * static_cast<f32>(Chunk::Data::chunk_size);

            ChunkRenderData &render_data = m_chunk_render_data[pos];

            auto &resources = m_renderer.get_resource_manager();
            resources.destroy_mesh(render_data.mesh);

            render_data.mesh =
                resources.upload_mesh(mesh.vertices, mesh.indices);

            render_data.model = glm::translate(glm::mat4{1.0f}, world_pos);

            render_data.aabb = {world_pos,
                                world_pos +
                                    static_cast<f32>(Chunk::Data::chunk_size)};

            processed++;
        }
    }

    void Engine::render() {
        glm::mat4 view = m_camera.view();
        glm::mat4 projection = m_camera.projection();

        m_renderer.set_camera(view, projection);
        m_frustum.extract(projection * view);

        Renderer::Result result = m_renderer.begin_frame();
        if (result == Renderer::Result::FrameNeedsResize) {
            auto [width, height] = m_window.get_framebuffer_size();
            m_renderer.resize(width, height);
            return;
        }

        m_renderer.begin_pass();

        for (const auto &data : m_chunk_render_data | std::views::values) {
            if (!m_frustum.intersects(data.aabb)) {
                continue;
            }
            result = m_renderer.draw_mesh(data.mesh, data.model);
            if (result == Renderer::Result::InvalidMeshHandle) {
                spdlog::error("Invalid mesh handle. Should be unreachable");
                break;
            }
        }
        m_renderer.end_pass();

        result = m_renderer.end_frame();
        if (result == Renderer::Result::FrameNeedsResize) {
            auto [width, height] = m_window.get_framebuffer_size();
            m_renderer.resize(width, height);
        }
    }
} // namespace mantle
