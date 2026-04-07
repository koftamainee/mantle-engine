#include <camera/camera.h>

#include "glm/gtc/matrix_transform.hpp"

namespace mantle {
    glm::mat4 Camera::view() const {
        return glm::lookAt(position, position + front, up);
    }

    glm::mat4 Camera::projection() const {
        glm::mat4 projection =
            glm::perspective(glm::radians(fov), aspect, near, far);
        projection[1][1] *= -1;
        return projection;
    }
} // namespace mantle
