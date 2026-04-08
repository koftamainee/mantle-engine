#include "math/plane.h"

#include <glm/glm.hpp>

namespace mantle {

    f32 Plane::signed_distance(glm::vec3 point) const {
        return glm::dot(normal, point) + distance;
    }
} // namespace mantle
