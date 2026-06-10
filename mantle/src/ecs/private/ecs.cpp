// Copyright (c) 2026 Mantle. All rights reserved.

#include "ecs/ecs.h"

#include "camera/camera.h"
#include "camera/components.h"
#include "core/assert.h"
#include "input/input.h"
#include "physics/character_controller.h"
#include "physics/physics_system.h"
#include "player/player.h"
#include "rumbling/rumbling.h"

namespace mantle {
    void Ecs::init(Window &window, f32 camera_aspect, CharacterController &character) {
        MANTLE_CHECK(!m_is_initialized);
        m_logger = spdlog::get("ecs").get();
        m_is_initialized = true;

        bootstrap_input(m_world, window);
        bootstrap_rumbling(m_world, window);
        bootstrap_camera(m_world, camera_aspect);
        bootstrap_player(m_world, character);

        m_logger->info("Ecs is initialized");
    }

    void Ecs::destroy() {
        if (m_is_initialized) {
            m_is_initialized = false;
            m_logger->info("Ecs is destroyed");
        }
    }

    void Ecs::update(f32 delta_time) {
        MANTLE_CHECK(m_is_initialized);
        static_cast<void>(m_world.progress(delta_time)); // xd
    }

    glm::mat4 Ecs::camera_view_proj() const { return m_world.get<Camera>().view_proj; }

    void Ecs::set_camera_aspect(f32 aspect) { m_world.get_mut<Camera>().aspect = aspect; }

} // namespace mantle
