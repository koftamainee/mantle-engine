// Copyright (c) 2026 Mantle. All rights reserved.
#pragma once

// fix-includes off
#include <Jolt/Jolt.h>
// fix-includes on

#include <Jolt/Physics/Character/CharacterVirtual.h>

#include "physics/character_controller.h"
#include "physics/physics_system.h"
#include "layers.h"

namespace mantle {

    struct CharacterController::Impl final {
        JPH::Ref<JPH::CharacterVirtual> character;
        PhysicsSystem                  *physics_system = nullptr;

        constexpr static f32 kCapsuleRadius = 0.3f;
        constexpr static f32 kCapsuleHalfHeight = 0.5f;
        constexpr static f32 kMaxSlopeAngle = JPH::DegreesToRadians(45.0f);

        explicit Impl(PhysicsSystem &physics, glm::vec3 start_pos);
        ~Impl() = default;
    };

} // namespace mantle
