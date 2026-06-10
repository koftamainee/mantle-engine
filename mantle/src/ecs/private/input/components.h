// Copyright (c) 2026 Mantle. All rights reserved.

#pragma once

#include "core/types.h"

namespace mantle {
    struct InputState {
        bool controller_active = false;
        f32  look_dx = 0.0f;
        f32  look_dy = 0.0f;
        f32  move_forward = 0.0f;
        f32  move_strafe = 0.0f;
        f32  move_up = 0.0f;
        bool sprint = false;
        bool jump = false;
    };
} // namespace mantle
