#include "world/chunk_generation_system.h"

#include <cstdlib>

namespace mantle {
    void ChunkGenerationSystem::generate(Chunk::Data &chunk,
                                         glm::ivec3 chunk_coord) {
        constexpr int SIZE = Chunk::Data::chunk_size;

        auto &voxels = chunk.voxels;

        for (int x = 0; x < SIZE; x++) {
            for (int y = 0; y < SIZE; y++) {
                for (int z = 0; z < SIZE; z++) {

                    int world_y = chunk_coord.y * SIZE + y;

                    uint16_t value = 0;

                    if (world_y > 0) {
                        if (std::rand() % 100 > 98) {
                            value = 1u;
                        } else {
                            value = 0u;
                        }
                    } else {
                        value = 1u;
                    }

                    voxels[index(x, y, z)] = value;
                }
            }
        }
    }
} // namespace mantle
