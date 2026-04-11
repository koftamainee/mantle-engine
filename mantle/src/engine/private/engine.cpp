#include "engine/engine.h"

#include <algorithm>
#include <ranges>

#include "camera/camera.h"
#include "core/assert.h"
#include "core/memory/memory_units.h"
#include "renderer/renderer.h"
#include "spdlog/spdlog.h"
#include "window/window.h"

namespace mantle {
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

        m_renderer.init(m_window, &m_heap, &m_scratch_arena);

        m_camera.aspect = static_cast<f32>(prop.size.width) /
            static_cast<f32>(prop.size.height);

        m_window.set_resize_callback([&](u32 w, u32 h) {
            m_renderer.resize(w, h);
            m_camera.aspect = static_cast<f32>(w) / static_cast<f32>(h);
        });

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
        if (m_window.is_key_just_pressed(Window::Key::Ctrl)) {
            m_camera_speed = m_base_camera_speed * 2;
        }
        if (m_window.is_key_just_released(Window::Key::Ctrl)) {
            m_camera_speed = m_base_camera_speed;
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

        //TODO: rendering

        m_renderer.end_pass();

        result = m_renderer.end_frame();
        if (result == Renderer::Result::FrameNeedsResize) {
            auto [width, height] = m_window.get_framebuffer_size();
            m_renderer.resize(width, height);
        }
    }
} // namespace mantle
