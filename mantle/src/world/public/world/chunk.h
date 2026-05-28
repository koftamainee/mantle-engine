#pragma once
#include "core/types.h"

namespace mantle {
    using Voxel = u16;

    struct Chunk {
        static constexpr u32 size = 32;
        static constexpr u32 volume = size * size * size;

        Voxel voxels[volume];
    };
}