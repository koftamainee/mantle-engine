// Copyright (c) 2026 Mantle. All rights reserved.

#pragma once

#include <glm/glm.hpp>

#include "mantle/renderer/types.h"

namespace mantle {
    struct BbBackbuffer {
        FGImageHandle handle;
    };
    struct BbCameraData {
        glm::mat4 view_proj;
    };
    struct BbFramebufferSize {
        u32 width;
        u32 height;
    };
} // namespace mantle
