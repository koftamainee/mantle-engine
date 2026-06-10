// Copyright (c) 2026 Mantle. All rights reserved.

#include "camera.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "components.h"
#include "input/components.h"

namespace mantle {

    void bootstrap_camera(const flecs::world &world, f32 aspect) {
        (void)world.component<Camera>();

        Camera cam {};
        cam.aspect = aspect;
        cam.fov = 75.0f;
        cam.position = glm::vec3(0.0f, 0.0f, 0.0f);

        world.set<Camera>(cam);

        world.system<>("Camera").kind(flecs::OnUpdate).run([](const flecs::iter &it) {
            auto w = it.world();

            auto       &camera = w.ensure<Camera>();
            const auto &input = w.get<InputState>();

            camera.yaw += input.look_dx;
            camera.pitch = glm::clamp(camera.pitch + input.look_dy, -89.0f, 89.0f);

            const float yawRad = glm::radians(camera.yaw);
            const float pitchRad = glm::radians(camera.pitch);

            glm::vec3 front;
            front.x = std::cos(pitchRad) * std::sin(yawRad);
            front.y = std::sin(pitchRad);
            front.z = -std::cos(pitchRad) * std::cos(yawRad);

            camera.front = glm::normalize(front);

            camera.right = glm::normalize(glm::cross(glm::vec3(0.0f, 1.0f, 0.0f), camera.front));

            glm::mat4 view = glm::lookAt(camera.position, camera.position + camera.front,
                                         glm::vec3(0.0f, 1.0f, 0.0f));

            glm::mat4 proj =
                glm::perspective(glm::radians(camera.fov), camera.aspect, 0.1f, 1000.0f);

            proj[1][1] *= -1.0f;

            camera.view_proj = proj * view;
        });
    }

} // namespace mantle
