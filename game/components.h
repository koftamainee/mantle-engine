#pragma once

#include "mantle/core/types.h"

namespace game {
    using mantle::f32;

    struct CameraState {
        f32 yaw   = 0.0f;
        f32 pitch = -15.0f;
        f32 orbit_distance = 8.0f;
    };
} // namespace game
