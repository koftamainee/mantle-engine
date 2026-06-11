// Copyright (c) 2026 Mantle. All rights reserved.

#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace mantle {
    inline glm::vec3 orbit_position(glm::vec3 target, f32 distance, f32 yaw, f32 pitch) {
        float yawRad = glm::radians(yaw);
        float pitchRad = glm::radians(pitch);
        glm::vec3 offset;
        offset.x = distance * std::cos(pitchRad) * std::sin(yawRad);
        offset.y = distance * std::sin(pitchRad);
        offset.z = -distance * std::cos(pitchRad) * std::cos(yawRad);
        return target + offset;
    }

    inline glm::mat4 orbit_view(glm::vec3 position, glm::vec3 target) {
        return glm::lookAt(position, target, glm::vec3(0.0f, 1.0f, 0.0f));
    }

    inline glm::mat4 perspective(f32 fov, f32 aspect, f32 near, f32 far) {
        glm::mat4 proj = glm::perspective(glm::radians(fov), aspect, near, far);
        proj[1][1] *= -1.0f;
        return proj;
    }

    inline glm::mat4 compute_view_proj(const glm::vec3 &camera_pos, f32 yaw, f32 pitch,
                                        f32 fov, f32 aspect, f32 near, f32 far) {
        glm::vec3 dir;
        float yawRad = glm::radians(yaw);
        float pitchRad = glm::radians(pitch);
        dir.x = std::cos(pitchRad) * std::sin(yawRad);
        dir.y = std::sin(pitchRad);
        dir.z = -std::cos(pitchRad) * std::cos(yawRad);
        glm::vec3 target = camera_pos + dir;
        glm::mat4 view = glm::lookAt(camera_pos, target, glm::vec3(0, 1, 0));
        glm::mat4 proj = glm::perspective(glm::radians(fov), aspect, near, far);
        proj[1][1] *= -1.0f;
        return proj * view;
    }
} // namespace mantle
