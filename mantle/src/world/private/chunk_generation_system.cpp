#include "world/chunk_generation_system.h"

#include <cmath>

#include "core/assert.h"
#include "spdlog/spdlog.h"

namespace mantle {

    ChunkGenerationSystem::~ChunkGenerationSystem() { destroy(); }

    void ChunkGenerationSystem::init(u32 seed) {
        MANTLE_CHECK(!m_is_initialized);
        m_logger = spdlog::get("world").get();
        m_cave_noise = {
            .noise_fn = simplex3,
            .seed = seed,
            .scale = 0.04f,
            .octaves = 5,
            .lacunarity = 2.0f,
            .gain = 0.5f,
        };

        m_warp_noise = {
            .noise_fn = simplex3,
            .seed = seed + 1,
            .scale = 0.02f,
            .octaves = 3,
            .lacunarity = 2.0f,
            .gain = 0.5f,
        };

        m_detail_noise = {
            .noise_fn = simplex3,
            .seed = seed + 2,
            .scale = 0.08f,
            .octaves = 4,
            .lacunarity = 2.0f,
            .gain = 0.5f,
        };

        m_moss_noise = {
            .noise_fn = simplex3,
            .seed = seed + 3,
            .scale = 0.06f,
            .octaves = 3,
            .lacunarity = 2.0f,
            .gain = 0.5f,
        };

        m_floor_noise = {
            .noise_fn = simplex2,
            .seed = seed + 4,
            .scale = 0.04f,
            .octaves = 3,
            .lacunarity = 2.0f,
            .gain = 0.5f,
        };

        m_is_initialized = true;
        m_logger->info("Chunk generation system initialized");
    }

    void ChunkGenerationSystem::destroy() {
        if (m_is_initialized) {
            m_logger->info("Chunk generation system destroyed");
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

                    f32 dx = world_pos.x - 16.0f;
                    f32 dy = (world_pos.y - 16.0f) * 1.2f;
                    f32 dz = world_pos.z - 16.0f;
                    f32 d = sqrtf(dx * dx + dy * dy + dz * dz);

                    f32 n = m_detail_noise.sample_warped(world_pos, m_warp_noise, 10.0f);
                    f32 density = d - 73.0f - n * 6.0f;

                    f32 floor_n = m_floor_noise.sample({world_pos.x, world_pos.z});
                    f32 floor_y = 8.0f + floor_n * 4.0f;
                    density = fmaxf(density, floor_y - world_pos.y);

                    auto voxel = static_cast<Voxel>(VoxelType::Air);
                    if (density >= 0.0f) {
                        f32 wall_n = m_detail_noise.sample(world_pos);
                        f32 moss_n = m_moss_noise.sample(world_pos);

                        if (moss_n > 0.25f && wall_n > 0.1f) {
                            u32 moss_type = 3 + static_cast<u32>(moss_n * 3.0f + wall_n) % 3;
                            voxel = static_cast<Voxel>(moss_type);
                        } else if (wall_n > 0.4f) {
                            voxel = static_cast<Voxel>(VoxelType::Limestone);
                        } else {
                            voxel = static_cast<Voxel>(VoxelType::Stone);
                        }
                    }

                    chunk.voxels[z * Chunk::size * Chunk::size + y * Chunk::size + x] = voxel;
                }
            }
        }
    }

} // namespace mantle
