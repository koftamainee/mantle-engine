#pragma once
#include <array>

#include "core/types.h"
#include "glm/vec3.hpp"


namespace mantle {
    struct Chunk final {
        struct Data final {
            static constexpr u32 chunk_size = 16;
            static constexpr u32 chunk_area = chunk_size * chunk_size;
            static constexpr u32 chunk_volume = chunk_area * chunk_size;
            std::array<u8, chunk_volume> voxels{};
        };

        Data data;

        glm::ivec3 position = {0, 0, 0};
        bool is_dirty = false;
    };


    constexpr int index(int x, int y, int z) {
        return x + Chunk::Data::chunk_size * (y + Chunk::Data::chunk_size * z);
    }
} // namespace mantle
