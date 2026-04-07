#include "world/chunk.h"

#include <mdspan>

namespace mantle {
    Chunk::Chunk(glm::ivec3 position) : m_position(position) {}
    Chunk::VoxelSpan Chunk::voxels() { return VoxelSpan(m_voxels.data()); }

    Chunk::ConstVoxelSpan Chunk::voxels() const {
        return ConstVoxelSpan(m_voxels.data());
    }

    Chunk::VoxelArray &Chunk::voxel_array() { return m_voxels; }

    glm::ivec3 Chunk::position() const {
        return m_position;
    }
} // namespace mantle
