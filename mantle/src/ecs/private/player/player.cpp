// Copyright (c) 2026 Mantle. All rights reserved.

#include "player.h"

#include <glm/glm.hpp>

#include "camera/components.h"
#include "input/components.h"
#include "physics/character_controller.h"

namespace mantle {
    namespace {
        constexpr f32 kMoveSpeed = 5.0f;
        constexpr f32 kSprintMult = 2.0f;
        constexpr f32 kJumpSpeed = 6.0f;
        constexpr f32 kGravity = -20.0f;

        constexpr f32 kEyeHeight = 1.6f;

        f32 s_vertical_velocity = 0.0f;
    } // namespace

    void bootstrap_player(const flecs::world &world, CharacterController &character) {
        world.system<>("Player").kind(flecs::OnUpdate).run([&character](const flecs::iter &it) {
            auto        w = it.world();
            const auto &input = w.get<InputState>();
            auto       &camera = w.ensure<Camera>();

            glm::vec3 forward = camera.front;
            forward.y = 0.0f;

            if (glm::length(forward) > 0.0f) {
                forward = glm::normalize(forward);
            }

            glm::vec3 right = camera.right;
            right.y = 0.0f;

            if (glm::length(right) > 0.0f) {
                right = glm::normalize(right);
            }

            const f32 speed = kMoveSpeed * (input.sprint ? kSprintMult : 1.0f);

            glm::vec3 velocity(0.0f);
            velocity += forward * input.move_forward * speed;
            velocity -= right * input.move_strafe * speed;

            if (character.is_grounded()) {
                s_vertical_velocity = input.jump ? kJumpSpeed : 0.0f;
            } else {
                s_vertical_velocity += kGravity * it.delta_time();
            }

            velocity.y = s_vertical_velocity;

            character.move(velocity, it.delta_time());

            camera.position = character.get_position() + glm::vec3(0.0f, kEyeHeight, 0.0f);
        });
    }

} // namespace mantle
