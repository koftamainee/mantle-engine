#include "math/aabb.h"

#include "core/assert.h"

namespace mantle {
    bool AABB::intersects(const AABB &other) const {
        if (max[0] < other.min[0] || min[0] > other.max[0]) {
            return false;
        }
        if (max[1] < other.min[1] || min[1] > other.max[1]) {
            return false;
        }
        if (max[2] < other.min[2] || min[2] > other.max[2]) {
            return false;
        }
        return true;
    }

    bool AABB::contains(glm::vec3 point) const {
        return (point.x >= min.x && point.x <= max.x) &&
            (point.y >= min.y && point.y <= max.y) &&
            (point.z >= min.z && point.z <= max.z);
    }

    glm::vec3 AABB::size() const { return max - min; }

    glm::vec3 AABB::center() const { return (max - min) * 0.5f; }

    void AABB::expand(glm::vec3 point) {
        min.x = std::min(min.x, point.x);
        min.y = std::min(min.y, point.y);
        min.z = std::min(min.z, point.z);

        max.x = std::max(max.x, point.x);
        max.y = std::max(max.y, point.y);
        max.z = std::max(max.z, point.z);
    }
} // namespace mantle
