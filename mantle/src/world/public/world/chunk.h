#pragma once
#include <cstdint>
#include <mdspan>
#include "voxel.h"

namespace mantle {
    class Chunk final {
    public:
        static constexpr uint32_t s_chunk_size = 16;
        using VoxelArray = std::array<Voxel, s_chunk_size * s_chunk_size * s_chunk_size>;
        using Extents = std::extents<uint32_t, s_chunk_size, s_chunk_size, s_chunk_size>;
        using VoxelSpan = std::mdspan<Voxel, Extents>;

        VoxelSpan voxels();
        VoxelArray voxel_array() const;

    private:

        VoxelArray m_voxels{};

    };
}
