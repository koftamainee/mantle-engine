#pragma once
#include <cstdint>

namespace mantle {
    struct MeshHandle {
        uint32_t id = 0;
        uint32_t generation = 0;

        bool is_valid() const { return generation != 0; }
    };
}