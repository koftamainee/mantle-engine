#include "world/chunk_meshing_system.h"
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

    Mesh ChunkMeshingSystem::build(const Chunk::Data& chunk) {
        Mesh mesh;

        auto in_bounds = [](glm::ivec3 pos) {
            return pos.x >= 0 && pos.x < Chunk::Data::chunk_size &&
                   pos.y >= 0 && pos.y < Chunk::Data::chunk_size &&
                   pos.z >= 0 && pos.z < Chunk::Data::chunk_size;
        };

        auto is_air = [&](glm::ivec3 pos) -> bool {
            return chunk.voxels[index(pos.x, pos.y, pos.z)] == 0;
        };

        for (i32 x = 0; x < Chunk::Data::chunk_size; x++) {
            for (i32 y = 0; y < Chunk::Data::chunk_size; y++) {
                for (i32 z = 0; z < Chunk::Data::chunk_size; z++) {

                    glm::ivec3 pos{x, y, z};

                    if (is_air(pos)) {
                        continue;
                    }

                    for (i8 dir = 0; dir < 6; dir++) {
                        glm::ivec3 neighbor_pos = pos + directions[dir];

                        bool visible = !in_bounds(neighbor_pos) ||
                                       is_air(neighbor_pos);

                        if (!visible) {
                            continue;
                        }

                        u32 base = static_cast<u32>(mesh.vertices.size());

                        auto fpos = glm::vec3(pos);

                        for (int v = 0; v < 4; v++) {
                            mesh.vertices.push_back({
                                fpos + faces[dir][v],
                                normals[dir]
                            });
                        }

                        mesh.indices.insert(mesh.indices.end(), {
                            base + 0, base + 1, base + 2,
                            base + 0, base + 2, base + 3
                        });
                    }
                }
            }
        }

        return mesh;
    }
} // namespace mantle
