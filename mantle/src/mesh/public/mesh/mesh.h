#pragma once
#include <vector>
#include <cstdint>

#include "vertex.h"

namespace mantle {
    struct Mesh {
        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;
    };
}