// Copyright (c) 2026 Mantle. All rights reserved.

#include "mantle/ecs/ecs.h"

#include "mantle/core/assert.h"
#include "mantle/ecs/components.h"

namespace mantle {
    void Ecs::init() {
        MANTLE_CHECK(!m_is_initialized);
        m_logger = spdlog::get("ecs").get();
        m_is_initialized = true;

        m_world.component<Transform>();
        m_world.component<MeshComponent>();
        m_world.component<Camera>();

        m_logger->info("Ecs is initialized");
    }

    void Ecs::destroy() {
        if (m_is_initialized) {
            m_is_initialized = false;
            m_logger->info("Ecs is destroyed");
        }
    }

    void Ecs::update(f32 dt) {
        MANTLE_CHECK(m_is_initialized);
        static_cast<void>(m_world.progress(dt));
    }

    Entity Ecs::create_entity(std::string_view name) {
        flecs::entity e = name.empty() ? m_world.entity() : m_world.entity(name.data());
        return Entity(e);
    }

    Entity Ecs::lookup(std::string_view name) const {
        return Entity(m_world.lookup(name.data()));
    }
} // namespace mantle
