#pragma once
#include "chunk.h"
#include "mesh/mesh.h"

namespace mantle {
    struct ChunkMeshingSystem final {
        static Mesh build(const Chunk::Data &chunk);
    };
} // namespace mantle
