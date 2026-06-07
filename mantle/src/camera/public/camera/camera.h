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
         * Default yaw is 0, so the camera looks towards Z-
         */
        f32 fov = 75.0f;
        f32 aspect = 16.0f / 9.0f;
        f32 z_near = 0.1f;
        f32 z_far = 1000.0f;

        glm::vec3 position = {0.0f, 0.0f, 0.0f};
        glm::vec3 front = glm::vec3(0.0f, 0.0f, -1.0f);
        glm::vec3 right = glm::vec3(1.0f, 0.0f, 0.0f);

        f32 yaw = 0.0f;
        f32 pitch = 0.0f;

        static constexpr glm::vec3 world_up = {0.0f, 1.0f, 0.0f};

      public:
        struct alignas(16) GPUData final {
            alignas(16) glm::vec3 position;
            alignas(16) glm::vec3 forward;
            alignas(16) glm::vec3 right;
            alignas(16) glm::vec3 up;
            f32 fov;
            f32 aspect;
            alignas(16) glm::mat4 view_proj;
        };

        GPUData gpu_data() const {
            return {
                .position = position,
                .forward = front,
                .right = right,
                .up = glm::cross(right, front),
                .fov = glm::radians(fov),
                .aspect = aspect,
                .view_proj = projection() * view(),
            };
        }

      public:
        glm::mat4 view() const;
        glm::mat4 projection() const;

        void rotate(f32 dx, f32 dy);
    };
} // namespace mantle
