#pragma once
#include "chunk.h"
#include "core/macros.h"
#include "glm/vec3.hpp"
#include "noise/sampler.h"

namespace mantle {
    enum class VoxelType : u16 {
        Air = 0,
        Dirt = 1,
        Stone = 2,
    };

    class ChunkGenerationSystem final {
    public:
        ChunkGenerationSystem() = default;
        ~ChunkGenerationSystem();

        MANTLE_NO_COPY_NO_MOVE(ChunkGenerationSystem);

        void init(u32 seed);
        void destroy();

        void generate(Chunk &chunk, glm::ivec3 pos) const;

    private:
        bool m_is_initialized = false;

        Sampler<f32 (*)(glm::vec3)> m_cave_noise{};
        Sampler<f32 (*)(glm::vec3)> m_warp_noise{};

    };
} // namespace mantle
