#pragma once
#include "core/types.h"

namespace mantle {
    struct Voxel final {
        u16 id;

        bool is_air() const { return id == 0; }
        Voxel() = default;
    };
} // namespace mantle
