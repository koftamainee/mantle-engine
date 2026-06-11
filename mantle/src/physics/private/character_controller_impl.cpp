// Copyright (c) 2026 Mantle. All rights reserved.

#include "character_controller_impl.h"

// fix-includes off
#include <Jolt/Jolt.h>
// fix-includes on

#include <Jolt/Physics/Character/CharacterVirtual.h>
#include <Jolt/Physics/Collision/Shape/CapsuleShape.h>
#include <Jolt/Physics/Collision/Shape/RotatedTranslatedShape.h>

#include "physics_system_impl.h"

namespace mantle {

    CharacterController::Impl::Impl(PhysicsSystem &physics, glm::vec3 start_pos) {
        physics_system = &physics;

        JPH::Ref<JPH::Shape> capsule =
            JPH::RotatedTranslatedShapeSettings(
                JPH::Vec3(0, kCapsuleRadius + kCapsuleHalfHeight, 0), JPH::Quat::sIdentity(),
                new JPH::CapsuleShape(kCapsuleHalfHeight, kCapsuleRadius))
                .Create()
                .Get();

        JPH::CharacterVirtualSettings settings;
        settings.mMaxSlopeAngle = kMaxSlopeAngle;
        settings.mShape = capsule;
        settings.mUp = JPH::Vec3::sAxisY();
        settings.mSupportingVolume = JPH::Plane(JPH::Vec3::sAxisY(), -kCapsuleRadius);

        character = new JPH::CharacterVirtual(
            &settings, JPH::RVec3(start_pos.x, start_pos.y, start_pos.z), JPH::Quat::sIdentity(), 0,
            &physics_system->m_impl->physics_system);
    }


    void CharacterController::init(PhysicsSystem &physics, glm::vec3 start_pos) {
        MANTLE_CHECK(!m_is_initialized);
        m_impl = physics.m_phys_allocator.emplace<Impl>(physics, start_pos);
        m_is_initialized = true;
    }

    void CharacterController::destroy() {
        if (m_is_initialized) {
            m_impl->~Impl();
            m_impl->physics_system->m_phys_allocator.free(m_impl);
            m_impl = nullptr;
            m_is_initialized = false;
        }
    }

    void CharacterController::move(glm::vec3 velocity, f32 dt) {
        MANTLE_CHECK(m_is_initialized);

        auto &jolt_system = m_impl->physics_system->m_impl->physics_system;

        m_impl->character->SetLinearVelocity({velocity.x, velocity.y, velocity.z});
        JPH::CharacterVirtual::ExtendedUpdateSettings update_settings;
        m_impl->character->ExtendedUpdate(
            dt, {gravity.x, gravity.y, gravity.z}, update_settings,
            jolt_system.GetDefaultBroadPhaseLayerFilter(Layers::MOVING),
            jolt_system.GetDefaultLayerFilter(Layers::MOVING), {}, {},
            m_impl->physics_system->m_impl->temp_allocator);
    }

    glm::vec3 CharacterController::get_position() const {
        MANTLE_CHECK(m_is_initialized);
        JPH::RVec3 position = m_impl->character->GetPosition();
        return {position.GetX(), position.GetY(), position.GetZ()};
    }

    bool CharacterController::is_grounded() const {
        MANTLE_CHECK(m_is_initialized);
        return m_impl->character->GetGroundState() == JPH::CharacterVirtual::EGroundState::OnGround;
    }

} // namespace mantle
