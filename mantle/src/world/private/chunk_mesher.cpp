#include "world/chunk_mesher.h"
namespace mantle {
    namespace {
        constexpr std::array<glm::ivec3, 6> directions = {{{1, 0, 0},
                                                           {-1, 0, 0},
                                                           {0, 1, 0},
                                                           {0, -1, 0},
                                                           {0, 0, 1},
                                                           {0, 0, -1}}};

        constexpr std::array<glm::vec3, 6> normals = {{{1, 0, 0},
                                                       {-1, 0, 0},
                                                       {0, 1, 0},
                                                       {0, -1, 0},
                                                       {0, 0, 1},
                                                       {0, 0, -1}}};

        constexpr std::array<std::array<glm::vec3, 4>, 6> faces = {{
            {{{1, 0, 1}, {1, 1, 1}, {1, 1, 0}, {1, 0, 0}}}, // +X
            {{{0, 0, 0}, {0, 1, 0}, {0, 1, 1}, {0, 0, 1}}}, // -X
            {{{0, 1, 0}, {1, 1, 0}, {1, 1, 1}, {0, 1, 1}}}, // +Y
            {{{0, 0, 1}, {1, 0, 1}, {1, 0, 0}, {0, 0, 0}}}, // -Y
            {{{0, 0, 1}, {0, 1, 1}, {1, 1, 1}, {1, 0, 1}}}, // +Z
            {{{1, 0, 0}, {1, 1, 0}, {0, 1, 0}, {0, 0, 0}}}, // -Z
        }};
    } // namespace

    Mesh ChunkMesher::build(const Chunk &chunk) {
        Mesh mesh;
        Chunk::ConstVoxelSpan voxels = chunk.voxels();

        auto in_bounds = [](glm::ivec3 pos) {
            return pos.x >= 0 && pos.x < Chunk::s_chunk_size && pos.y >= 0 &&
                pos.y < Chunk::s_chunk_size && pos.z >= 0 &&
                pos.z < Chunk::s_chunk_size;
        };

        for (u32 x = 0; x < Chunk::s_chunk_size; x++) {
            for (u32 y = 0; y < Chunk::s_chunk_size; y++) {
                for (u32 z = 0; z < Chunk::s_chunk_size; z++) {
                    if (voxels[x, y, z].is_air()) {
                        continue;
                    }

                    glm::ivec3 pos{x, y, z};

                    for (int direction = 0; direction < 6; direction++) {
                        glm::ivec3 neighbor_pos = pos + directions[direction];
                        bool is_visible = !in_bounds(neighbor_pos) ||
                            voxels[neighbor_pos.x, neighbor_pos.y,
                                   neighbor_pos.z]
                                .is_air();

                        if (!is_visible) {
                            continue;
                        }

                        u32 base = mesh.vertices.size();
                        glm::vec3 fpos{pos};

                        for (int vert = 0; vert < 4; vert++) {
                            mesh.vertices.push_back(
                                {fpos + faces[direction][vert],
                                 normals[direction]});
                        }

                        mesh.indices.insert(mesh.indices.end(),
                                            {base + 0, base + 1, base + 2,
                                             base + 0, base + 2, base + 3});
                    }
                }
            }
        }
        return mesh;
    }
} // namespace mantle
