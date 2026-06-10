// Copyright (c) 2026 Mantle. All rights reserved.

#pragma once

#include <array>

#include <glm/glm.hpp>

#include "aabb.h"
#include "plane.h"

namespace mantle {
    struct Frustum final {

        void extract(const glm::mat4 &vp);
        bool intersects(const AABB &aabb) const;

      private:
        enum class Side {
            Left,
            Right,
            Bottom,
            Top,
            Near,
            Far,
        };
        std::array<Plane, 6> m_planes {};
    };
} // namespace mantle
