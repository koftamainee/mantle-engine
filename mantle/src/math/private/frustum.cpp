#include "math/frustum.h"

#include <utility>

namespace mantle {

    void Frustum::extract(const glm::mat4 &vp) {
        auto row = [&](int i) -> glm::vec4 {
            return {vp[0][i], vp[1][i], vp[2][i], vp[3][i]};
        };

        glm::vec4 r0 = row(0), r1 = row(1), r2 = row(2), r3 = row(3);

        auto make_plane = [](glm::vec4 v) -> Plane {
            float len = glm::length(glm::vec3(v));
            return {glm::vec3(v) / len, v.w / len};
        };

        auto index = [](Side side) { return std::to_underlying(side); };

        m_planes[index(Side::Left)] = make_plane(r3 + r0);
        m_planes[index(Side::Right)] = make_plane(r3 - r0);
        m_planes[index(Side::Bottom)] = make_plane(r3 + r1);
        m_planes[index(Side::Top)] = make_plane(r3 - r1);
        m_planes[index(Side::Near)] = make_plane(r3 + r2);
        m_planes[index(Side::Far)] = make_plane(r3 - r2);
    }

    bool Frustum::intersects(const AABB &aabb) const {
        for (const Plane &plane : m_planes) {
            glm::vec3 point = aabb.min;
            if (plane.normal.x >= 0) {
                point.x = aabb.max.x;
            }
            if (plane.normal.y >= 0) {
                point.y = aabb.max.y;
            }
            if (plane.normal.z >= 0) {
                point.z = aabb.max.z;
            }

            if (plane.signed_distance(point) < 0.0f) {
                return false;
            }
        }
        return true;
    }
} // namespace mantle
