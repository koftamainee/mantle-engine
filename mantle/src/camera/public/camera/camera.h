#pragma once
#include <glm/glm.hpp>

#include "core/types.h"

namespace mantle {
    struct Camera final {
        glm::vec3 position = {0.0f, 0.0f, 0.0f};
        glm::vec3 front = glm::vec3(0.0f, 0.0f, -1.0f);
        glm::vec3 right = glm::vec3(1.0f, 0.0f, 0.0f);

        f32 yaw = -90.0f;
        f32 pitch = 0.0f;

        static constexpr glm::vec3 up = {0.0f, 1.0f, 0.0f};

        f32 fov = 75.0f;
        f32 aspect = 16.0f / 9.0f;
        f32 near = 0.1f;
        f32 far = 1000.0f;

        glm::mat4 view() const;
        glm::mat4 projection() const;
    };
} // namespace mantle
