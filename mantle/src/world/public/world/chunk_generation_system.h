#pragma once
#include "chunk.h"

namespace mantle {
    struct ChunkGenerationSystem final {
        static void generate(Chunk::Data &chunk, glm::ivec3 chunk_coord);
    };
}