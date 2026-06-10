// Copyright (c) 2026 Mantle. All rights reserved.

#include "physics/physics_system.h"

#include <cstdarg>

// fix-includes off
#include <Jolt/Jolt.h>
// fix-includes on

#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>

#include "core/assert.h"
#include "core/memory/memory_units.h"
#include "physics_system_impl.h"

namespace mantle {
    void PhysicsSystem::init(MemoryBlock block) {
        MANTLE_CHECK(!m_is_initialized);

        m_impl = new Impl(block); // fuck it we use global heap
        m_is_initialized = true;
        m_impl->logger->info("Physics system initialized");
    }


    void PhysicsSystem::update(f32 dt) {
        MANTLE_CHECK(m_is_initialized);

        // m_impl->temp_allocator.Reset();

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

    void PhysicsSystem::add_static_box(glm::vec3 pos, glm::vec3 half_extents) {
        MANTLE_CHECK(m_is_initialized);
        auto &body_interface = m_impl->physics_system.GetBodyInterface();

        JPH::BoxShapeSettings shape_settings(
            JPH::Vec3(half_extents.x, half_extents.y, half_extents.z));

        JPH::Shape::ShapeResult shape_result = shape_settings.Create();
        MANTLE_CHECK(shape_result.IsValid());
        JPH::Ref<JPH::Shape> shape = shape_result.Get();

        JPH::BodyCreationSettings body_settings(shape, JPH::RVec3(pos.x, pos.y, pos.z),
                                                JPH::Quat::sIdentity(), JPH::EMotionType::Static,
                                                Layers::NON_MOVING);
        body_interface.CreateAndAddBody(body_settings, JPH::EActivation::DontActivate);
    }

} // namespace mantle
