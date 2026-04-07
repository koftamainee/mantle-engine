#pragma once
#include "chunk.h"
#include "mesh/mesh.h"

namespace mantle {
    class ChunkMesher {
    public:
        static Mesh build(const Chunk &chunk);
    };
}