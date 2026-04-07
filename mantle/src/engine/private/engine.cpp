#include "engine/engine.h"

#include <algorithm>

#include "camera/camera.h"
#include "glm/gtx/transform.hpp"
#include "mesh/mesh.h"
#include "renderer/renderer.h"
#include "spdlog/spdlog.h"
#include "window/window.h"
#include "world/chunk_mesher.h"
#include "world/world.h"

namespace mantle {
    void Engine::init() {
        Window::Properties prop = {
            .title = "Mantle",
            .size = {.width = 2560, .height = 1600},
        };
        m_window.init(prop);
        m_renderer.init(m_window);
        m_world.init();

        m_camera.aspect = 2560.0f / 1600.0f;

        m_window.set_resize_callback([&](u32 w, u32 h) {
            m_renderer.resize(w, h);
            m_camera.aspect = static_cast<f32>(w) / static_cast<f32>(h);
        });

        GPUResourceManager &resources = m_renderer.get_resource_manager();

        auto chunk_mesher = [&](Chunk &chunk) {
            chunk.is_dirty = false;
            Mesh mesh = ChunkMesher::build(chunk);
            if (mesh.vertices.empty()) {
                return;
            }

            glm::vec3 world_pos = chunk.world_pos();
            world_pos = world_pos * static_cast<f32>(Chunk::s_chunk_size);

            glm::mat4 model = glm::translate(glm::mat4{1.0f}, world_pos);

            m_models.emplace_back(model);

            m_meshes.push_back(
                resources.upload_mesh(mesh.vertices, mesh.indices));
        };

        for (i32 x = -2; x < 2; x++) {
            for (i32 y = -2; y < 0; y++) {
                for (i32 z = -2; z < 2; z++) {
                    m_world.generate_chunk({x, y, z});
                }
            }
        }

        m_world.for_each_chunk(chunk_mesher);

        m_last_time = 0;
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
        m_renderer.destroy();
        m_window.destroy();
    }

    void Engine::update(f32 delta_time) {
        m_window.update();


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
            m_camera.position -=
                Camera::up * m_camera_speed * delta_time;
        }
        if (m_window.is_key_pressed(Window::Key::Space)) {
            m_camera.position +=
                Camera::up * m_camera_speed * delta_time;
        }

        auto [mouse_x, mouse_y] = m_window.get_mouse_position();

        float dx = m_mouse_x - mouse_x;
        float dy = m_mouse_y - mouse_y;
        m_mouse_x = mouse_x;
        m_mouse_y = mouse_y;
        dx *= m_mouse_sensitivity;
        dy *= m_mouse_sensitivity;

        m_camera.yaw += dx;
        m_camera.pitch += dy;

        m_camera.pitch = std::clamp(m_camera.pitch, -89.0f, 89.0f);

        m_camera.front.x = cos(glm::radians(m_camera.yaw)) * cos(glm::radians(m_camera.pitch));
        m_camera.front.y = sin(glm::radians(m_camera.pitch));
        m_camera.front.z = -sin(glm::radians(m_camera.yaw)) * cos(glm::radians(m_camera.pitch));


        m_camera.front = glm::normalize(m_camera.front);
        m_camera.right = glm::normalize(glm::cross(m_camera.front, Camera::up));
    }

    void Engine::render() {
        m_renderer.set_camera(m_camera.view(), m_camera.projection());

        Renderer::Result result = m_renderer.begin_frame();
        if (result == Renderer::Result::FrameNeedsResize) {
            auto [width, height] = m_window.get_framebuffer_size();
            m_renderer.resize(width, height);
            return;
        }

        m_renderer.begin_pass();

        for (i32 i = 0; i < m_meshes.size(); i++) {
            result = m_renderer.draw_mesh(m_meshes[i], m_models[i]);
            if (result == Renderer::Result::InvalidMeshHandle) {
                spdlog::critical("Invalid mesh handle. Should be unreachable");
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
