#pragma once
#include <vector>
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/hash.hpp"


#include "chunk.h"

namespace mantle {
    struct ChunkStorage final {
        std::vector<Chunk> chunks;
        std::unordered_map<glm::ivec3, uint32_t> index;
    };
} // namespace mantle
