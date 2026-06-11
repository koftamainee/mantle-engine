// Copyright (c) 2026 Mantle. All rights reserved.

#include "mantle/noise/sampler.h"

namespace mantle::detail {
    glm::vec2 seed_offset(u32 seed) {
        return glm::vec2 {
            static_cast<f32>(seed) * 31.7f,
            static_cast<f32>(seed) * 73.1f,
        };
    }

    glm::vec3 seed_offset_3d(u32 seed) {
        return glm::vec3 {
            static_cast<f32>(seed) * 31.7f,
            static_cast<f32>(seed) * 73.1f,
            static_cast<f32>(seed) * 97.3f,
        };
    }
} // namespace mantle::detail
