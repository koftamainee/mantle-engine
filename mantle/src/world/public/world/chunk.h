#pragma once
#include <cstdint>
#include <mdspan>
#include "glm/vec3.hpp"
#include "voxel.h"

namespace mantle {
    class Chunk final {
      public:
        static constexpr uint32_t s_chunk_size = 16;
        using VoxelArray =
            std::array<Voxel, s_chunk_size * s_chunk_size * s_chunk_size>;
        using Extents =
            std::extents<uint32_t, s_chunk_size, s_chunk_size, s_chunk_size>;
        using VoxelSpan = std::mdspan<Voxel, Extents>;
        using ConstVoxelSpan = std::mdspan<const Voxel, Extents>;

      public:
        explicit Chunk(glm::ivec3 position);

        VoxelSpan voxels();
        ConstVoxelSpan voxels() const;
        VoxelArray &voxel_array();

        glm::ivec3 world_pos() const;

        bool is_dirty = true;

      private:
        glm::ivec3 m_position{};
        VoxelArray m_voxels{};
    };
} // namespace mantle
