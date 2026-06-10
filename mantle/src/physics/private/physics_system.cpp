// Copyright (c) 2026 Mantle. All rights reserved.

#include "physics/physics_system.h"

#include <cstdarg>

// fix-includes off
#include <Jolt/Jolt.h>
// fix-includes on

#include "physics_system_impl.h"

#include "core/assert.h"
#include "core/memory/memory_units.h"

namespace mantle {
    void PhysicsSystem::init(MemoryBlock block) {
        MANTLE_CHECK(!m_is_initialized);

        m_impl = new Impl(block); // fuck it we use global heap
        m_is_initialized = true;
        m_impl->logger->info("Physics system initialized");
    }


    void PhysicsSystem::update(f32 dt) {
        MANTLE_CHECK(m_is_initialized);
        m_impl->physics_system.Update(dt, m_impl->kCollisionSteps, &m_impl->temp_allocator,
                                      &m_impl->job_system);
    }

    void PhysicsSystem::destroy() {
        if (m_is_initialized) {
            m_is_initialized = false;

            delete m_impl;

            spdlog::get("physics")->info("Physics system destroyed");
        }
    }

} // namespace mantle
