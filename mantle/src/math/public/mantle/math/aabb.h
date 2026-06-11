// Copyright (c) 2026 Mantle. All rights reserved.

#pragma once

#include <glm/vec3.hpp>

namespace mantle {
    struct AABB final {
        glm::vec3 min;
        glm::vec3 max;

        bool intersects(const AABB &other) const;
        bool contains(glm::vec3 point) const;

        glm::vec3 size() const;
        glm::vec3 center() const;

        void expand(glm::vec3 point);
    };
} // namespace mantle
