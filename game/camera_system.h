#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <algorithm>
#include <flecs.h>

#include "actions.h"
#include "components.h"
#include "mantle/engine/engine.h"
#include "mantle/input/input_system.h"

namespace game {

    inline void register_fly_camera_system(flecs::world &world) {
        world.system<FlyCamera>("FlyCamera")
            .kind(flecs::OnUpdate)
            .each([&world](flecs::entity, FlyCamera &cam) {
                auto *engine = mantle::Engine::instance();
                if (!engine) return;
                auto &input = engine->input_system();

                f32 dt = world.delta_time();

                f32 look_x = input.get_axis(GameAction::LookX);
                f32 look_y = input.get_axis(GameAction::LookY);
                cam.yaw   += look_x * cam.mouse_sensitivity;
                cam.pitch += look_y * cam.mouse_sensitivity;
                cam.pitch  = std::clamp(cam.pitch, -89.0f, 89.0f);

                f32 yaw_rad   = glm::radians(cam.yaw);
                f32 pitch_rad = glm::radians(cam.pitch);

                glm::vec3 forward;
                forward.x = sin(yaw_rad) * cos(pitch_rad);
                forward.y = sin(pitch_rad);
                forward.z = -cos(yaw_rad) * cos(pitch_rad);
                forward = glm::normalize(forward);

                glm::vec3 world_up(0.0f, 1.0f, 0.0f);
                glm::vec3 right = glm::normalize(glm::cross(forward, world_up));
                glm::vec3 up    = glm::normalize(glm::cross(right, forward));

                f32 speed = cam.speed * dt;
                glm::vec3 pos = cam.position;

                f32 forward_amount = input.get_axis(GameAction::MoveForward);
                f32 strafe_amount  = input.get_axis(GameAction::StrafeRight);
                f32 up_amount      = input.get_axis(GameAction::MoveUp);

                pos += forward * forward_amount * speed;
                pos += right * strafe_amount * speed;
                pos += world_up * up_amount * speed;
                cam.position = pos;

                glm::vec3 target = pos + forward;
                glm::mat4 view = glm::lookAt(pos, target, world_up);

                f32 aspect = static_cast<f32>(engine->window_width()) /
                             static_cast<f32>(std::max(engine->window_height(), 1u));
                glm::mat4 proj = glm::perspective(glm::radians(75.0f), aspect, 0.1f, 10000.0f);
                proj[1][1] *= -1.0f;

                engine->set_camera(proj * view, pos);
            });
    }

} // namespace game
