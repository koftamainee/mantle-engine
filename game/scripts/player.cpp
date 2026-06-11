#include "player.h"

#include <glm/glm.hpp>

#include "mantle/ecs/ecs.h"
#include "mantle/ecs/components.h"
#include "mantle/engine/engine.h"
#include "mantle/input/input.h"
#include "mantle/physics/character_controller.h"

#include "../components.h"

namespace game {
    using mantle::f32;

    namespace {
        constexpr f32 kMoveSpeed = 5.0f;
        constexpr f32 kSprintMult = 2.0f;
        constexpr f32 kJumpSpeed = 6.0f;
        constexpr f32 kGravity = -20.0f;

        f32 s_vertical_velocity = 0.0f;

        void on_physics_update(mantle::Engine &engine, f32 dt) {
            auto  &ecs = engine.ecs();
            auto  *cam_state = ecs.get_singleton<CameraState>();
            auto  &character = engine.character();

            const float yawRad = glm::radians(cam_state ? cam_state->yaw : 0.0f);
            glm::vec3 forward(-std::sin(yawRad), 0.0f, std::cos(yawRad));
            glm::vec3 right(std::cos(yawRad), 0.0f, std::sin(yawRad));

            glm::vec2 move = mantle::Input::get_vector("move_left", "move_right", "move_forward", "move_backward");
            f32 speed = kMoveSpeed * (mantle::Input::is_action_pressed("sprint") ? kSprintMult : 1.0f);

            glm::vec3 velocity(0.0f);
            velocity += forward * move.y * speed;
            velocity += right * move.x * speed;

            if (character.is_grounded()) {
                s_vertical_velocity = mantle::Input::is_action_pressed("jump") ? kJumpSpeed : 0.0f;
            } else {
                s_vertical_velocity += kGravity * dt;
            }

            velocity.y = s_vertical_velocity;

            character.move(velocity, dt);
        }

        void on_update(mantle::Engine &engine, f32 dt) {
            (void)dt;
            auto &ecs = engine.ecs();
            auto &character = engine.character();

            mantle::Entity e = ecs.lookup("Capsule");
            if (e.is_alive()) {
                e.get_mut<mantle::Transform>()->position =
                    character.get_position() + glm::vec3(0.0f, 0.8f, 0.0f);
            }
        }
    }

    const mantle::ScriptCallbacks player_script = {
        .on_update         = on_update,
        .on_physics_update = on_physics_update,
    };
}
