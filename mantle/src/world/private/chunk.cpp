#include "world/chunk.h"

#include <mdspan>

namespace mantle {
    Chunk::VoxelSpan Chunk::voxels(){
        return VoxelSpan(m_voxels.data());
    }

    Chunk::VoxelArray Chunk::voxel_array() const {
        return m_voxels;
    }
}
