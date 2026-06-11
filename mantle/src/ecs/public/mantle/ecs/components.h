// Copyright (c) 2026 Mantle. All rights reserved.

#pragma once

#include <glm/glm.hpp>

#include "mantle/core/types.h"

namespace mantle {
    struct Transform {
        glm::vec3 position{0.0f};
        glm::vec3 rotation{0.0f};
        glm::vec3 scale{1.0f};
    };

    struct MeshComponent {
        u32 mesh_id = UINT32_MAX;
    };

    struct Camera {
        f32 fov    = 75.0f;
        f32 aspect = 1.0f;
        f32 near   = 0.1f;
        f32 far    = 1000.0f;
    };

} // namespace mantle
