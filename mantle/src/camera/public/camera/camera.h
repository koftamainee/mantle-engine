#pragma once
#include <glm/glm.hpp>

#include "core/types.h"

namespace mantle {
    struct Camera final {
        /* We use RH coordinates system in world space
         * X+ is right
         * Y+ is up
         * Z- is forward
         *
         * Default camera yaw is -90 deg, so it's looking towards Z-
         */
        f32 fov = 75.0f;
        f32 aspect = 16.0f / 9.0f;
        f32 z_near = 0.1f;
        f32 z_far = 1000.0f;

        glm::vec3 position = {0.0f, 0.0f, 0.0f};
        glm::vec3 front = glm::vec3(0.0f, 0.0f, -1.0f);
        glm::vec3 right = glm::vec3(1.0f, 0.0f, 0.0f);

        f32 yaw = -90.0f;
        f32 pitch = 0.0f;

        static constexpr glm::vec3 world_up = {0.0f, 1.0f, 0.0f};

    public:
        glm::mat4 view() const;
        glm::mat4 projection() const;

        void rotate(f32 dx, f32 dy);
    };
} // namespace mantle
