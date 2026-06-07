#include <camera/camera.h>

#include <algorithm>

#include "glm/gtc/matrix_transform.hpp"

namespace mantle {
    glm::mat4 Camera::view() const {
        return glm::lookAt(position, position + front, world_up);
    }

    glm::mat4 Camera::projection() const {
        // glm::perspective calling RH_ZO version. check root cmake
        glm::mat4 projection =
            glm::perspective(glm::radians(fov), aspect, z_near, z_far);
        projection[1][1] *= -1;
        // y in vulkan is flipped in NDC, so to follow math / opengl convention
        // in our code we need to flip it manually
        return projection;
    }
    void Camera::rotate(f32 dx, f32 dy) {
        yaw += dx;
        pitch += dy;

        pitch = std::clamp(pitch, -89.0f, 89.0f);

        front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        front.y = sin(glm::radians(pitch));
        front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));

        front = glm::normalize(front);
        right = glm::normalize(glm::cross(front, Camera::world_up));
    }
} // namespace mantle
