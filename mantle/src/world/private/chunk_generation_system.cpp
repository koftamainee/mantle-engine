#include "world/chunk_generation_system.h"

#include "core/assert.h"
#include "spdlog/spdlog.h"

namespace mantle {

    ChunkGenerationSystem::~ChunkGenerationSystem() { destroy(); }

    void ChunkGenerationSystem::init(u32 seed) {
        check(!m_is_initialized);
        m_cave_noise = {
            .noise_fn = simplex3,
            .seed = seed,
            .scale = 0.08f,
            .octaves = 4,
            .lacunarity = 2.0f,
            .gain = 0.5f,
        };

        m_warp_noise = {
            .noise_fn = simplex3,
            .seed = seed + 1,
            .scale = 0.03f,
            .octaves = 3,
            .lacunarity = 2.0f,
            .gain = 0.5f,
        };

        m_is_initialized = true;
        spdlog::info("Chunk generation system is initialized");
    }

    void ChunkGenerationSystem::destroy() {
        if (m_is_initialized) {
            spdlog::info("Chunk generation system is destroyed");
            m_is_initialized = false;
        }
    }

    void ChunkGenerationSystem::generate(Chunk &chunk, glm::ivec3 pos) const {
        for (u32 z = 0; z < Chunk::size; z++) {
            for (u32 y = 0; y < Chunk::size; y++) {
                for (u32 x = 0; x < Chunk::size; x++) {
                    glm::vec3 world_pos = {
                        static_cast<f32>(pos.x * static_cast<i32>(Chunk::size) + static_cast<i32>(x)),
                        static_cast<f32>(pos.y * static_cast<i32>(Chunk::size) + static_cast<i32>(y)),
                        static_cast<f32>(pos.z * static_cast<i32>(Chunk::size) + static_cast<i32>(z)),
                    };

                    f32 noise = m_cave_noise.sample_warped(world_pos, m_warp_noise, 15.0f);

                    Voxel voxel;
                    if (noise > 0.08f) {
                        voxel = static_cast<Voxel>(VoxelType::Air);
                    } else {
                        voxel = static_cast<Voxel>(VoxelType::Stone);
                    }

                    chunk.voxels[z * Chunk::size * Chunk::size + y * Chunk::size + x] = voxel;
                }
            }
        }
    }

} // namespace mantle
