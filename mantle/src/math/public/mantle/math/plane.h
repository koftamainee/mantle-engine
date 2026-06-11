// Copyright (c) 2026 Mantle. All rights reserved.

#pragma once

#include <glm/vec3.hpp>

#include "mantle/core/types.h"

namespace mantle {
    struct Plane final {
        glm::vec3 normal;
        f32       distance;

        f32 signed_distance(glm::vec3 point) const;
    };
} // namespace mantle
