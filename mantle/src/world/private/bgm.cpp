#include "bgm.h"

#include <bit>

#include "core/assert.h"
#include "world/face_mask.h"

namespace mantle::bgm {

    namespace {

        constexpr u32 CHUNK_SIZE = Chunk::size;
        constexpr f32 VOXEL_SCALE = 0.1f;


        u32 voxel_index(u32 axis, u32 a, u32 u, u32 v) noexcept {
            switch (axis) {
            case 0:
                return a + u * 32 + v * 1024;
            case 1:
                return u + a * 32 + v * 1024;
            default:
                return u + v * 32 + a * 1024;
            }
        }

        u16 sample_voxel(const Chunk &chunk, const Chunk *neighbors[6],
                         u32 axis, i32 a, u32 u, u32 v) noexcept {
            if (a >= 0 && a < static_cast<i32>(CHUNK_SIZE)) {
                auto idx = voxel_index(axis, static_cast<u32>(a), u, v);
                return chunk.voxels[idx];
            }

            u32 nb_idx = (axis * 2) + (a < 0 ? 0u : 1u);
            const Chunk *nb = neighbors[nb_idx];
            if (!nb) {
                return 0;
            }

            u32 na = (a < 0) ? CHUNK_SIZE - 1 : 0;
            return nb->voxels[voxel_index(axis, na, u, v)];
        }

        u16 sample_voxel_safe(const Chunk &chunk, const Chunk *neighbors[6],
                              u32 axis, i32 a, i32 u, i32 v) noexcept {
            bool a_oob = a < 0 || a >= static_cast<i32>(CHUNK_SIZE);
            bool u_oob = u < 0 || u >= static_cast<i32>(CHUNK_SIZE);
            bool v_oob = v < 0 || v >= static_cast<i32>(CHUNK_SIZE);

            u32 oob_count = (a_oob ? 1u : 0u) + (u_oob ? 1u : 0u) + (v_oob ? 1u : 0u);
            if (oob_count > 1) return 0;

            if (a_oob) {
                return sample_voxel(chunk, neighbors, axis, a,
                                    static_cast<u32>(u), static_cast<u32>(v));
            }

            if (u_oob) {
                u32 nb_idx;
                if (axis == 0) nb_idx = 2;  // u=Y
                else nb_idx = 0;             // u=X
                nb_idx += (u < 0 ? 0u : 1u);

                const Chunk *nb = neighbors[nb_idx];
                if (!nb) return 0;

                u32 nu = (u < 0) ? CHUNK_SIZE - 1 : 0;
                return nb->voxels[voxel_index(axis, static_cast<u32>(a), nu,
                                              static_cast<u32>(v))];
            }

            if (v_oob) {
                u32 nb_idx;
                if (axis <= 1) nb_idx = 4;  // v=Z
                else nb_idx = 2;             // v=Y
                nb_idx += (v < 0 ? 0u : 1u);

                const Chunk *nb = neighbors[nb_idx];
                if (!nb) return 0;

                u32 nv = (v < 0) ? CHUNK_SIZE - 1 : 0;
                return nb->voxels[voxel_index(axis, static_cast<u32>(a),
                                              static_cast<u32>(u), nv)];
            }

            return chunk.voxels[voxel_index(axis, static_cast<u32>(a),
                                            static_cast<u32>(u), static_cast<u32>(v))];
        }

        u32 vertex_ao(const Chunk &chunk, const Chunk *neighbors[6],
                      u32 axis, u32 positive, i32 a,
                      i32 u_c, i32 v_c) noexcept {
            i32 na = positive ? a + 1 : a;

            u16 diag = sample_voxel_safe(chunk, neighbors, axis, na,
                                         u_c - 1, v_c - 1);
            u16 side_u = sample_voxel_safe(chunk, neighbors, axis, na,
                                           u_c, v_c - 1);
            u16 side_v = sample_voxel_safe(chunk, neighbors, axis, na,
                                           u_c - 1, v_c);

            return (diag != 0 ? 1u : 0u) + (side_u != 0 ? 1u : 0u) + (side_v != 0 ? 1u : 0u);
        }

        void build_solid_masks(const Chunk &chunk, u32 axis,
                                      FaceMask solid[CHUNK_SIZE]) noexcept {
            for (u32 a = 0; a < CHUNK_SIZE; a++) {
                for (u32 v = 0; v < CHUNK_SIZE; v++) {
                    u32 bits = 0;
                    for (u32 u = 0; u < CHUNK_SIZE; u++) {
                        if (chunk.voxels[voxel_index(axis, a, u, v)] != 0) {
                            bits |= (1u << u);
                        }
                    }
                    solid[a].rows[v] = bits;
                }
            }
        }

        void build_face_masks(const Chunk *neighbors[], u32 axis, u32 positive,
                              const FaceMask solid[],
                              FaceMask face[]) noexcept {
            for (u32 a = 0; a < CHUNK_SIZE; a++) {
                i32 nb_a = static_cast<i32>(positive ? a + 1 : a - 1);

                FaceMask neighbor_mask = {};
                if (nb_a >= 0 && nb_a < static_cast<i32>(CHUNK_SIZE)) {
                    neighbor_mask = solid[nb_a];
                } else {
                    u32 nb_idx = axis * 2 + positive;
                    if (const Chunk *nb = neighbors[nb_idx]) {
                        u32 na = positive ? 0 : CHUNK_SIZE - 1;
                        for (u32 v = 0; v < CHUNK_SIZE; v++) {
                            u32 r = 0;
                            for (u32 u = 0; u < CHUNK_SIZE; u++) {
                                if (nb->voxels[voxel_index(axis, na, u, v)] !=
                                    0)
                                    r |= (1u << u);
                            }
                            neighbor_mask.rows[v] = r;
                        }
                    }
                }

                for (u32 v = 0; v < CHUNK_SIZE; v++) {
                    face[a].rows[v] = solid[a].rows[v] & ~neighbor_mask.rows[v];
                }
            }
        }

        glm::vec3 to_world_offset(u32 axis, f32 a, f32 u,
                                         f32 v) noexcept {
            switch (axis) {
            case 0:
                return {a, u, v};
            case 1:
                return {u, a, v};
            default:
                return {u, v, a};
            }
        }

        void emit_quad(ChunkMeshData &out, glm::vec3 chunk_world_origin,
                              u32 axis, u32 positive, u32 a, u32 u0, u32 v0,
                              u32 width, u32 height, u16 material,
                              const Chunk &chunk,
                              const Chunk *neighbors[6]) noexcept {
            u32 face_index = axis * 2 + positive;
            i32 ai = static_cast<i32>(a);
            i32 u0i = static_cast<i32>(u0);
            i32 v0i = static_cast<i32>(v0);
            i32 wi = static_cast<i32>(width);
            i32 hi = static_cast<i32>(height);

            i32 u_c[4] = {u0i, u0i + wi, u0i + wi, u0i};
            i32 v_c[4] = {v0i, v0i, v0i + hi, v0i + hi};

            u32 ao[4];
            for (u32 i = 0; i < 4; i++) {
                ao[i] = vertex_ao(chunk, neighbors, axis, positive, ai,
                                  u_c[i], v_c[i]);
            }

            f32 depth = static_cast<f32>(a) + (positive ? 1.f : 0.f);
            f32 W = static_cast<f32>(width);
            f32 H = static_cast<f32>(height);

            bool ccw = positive;
            if (axis == 1) {
                ccw = !ccw;
            }

            glm::vec3 corners[4];
            if (ccw) {
                corners[0] = to_world_offset(axis, depth, static_cast<f32>(u0), static_cast<f32>(v0));
                corners[1] = to_world_offset(axis, depth, static_cast<f32>(u0) + W, static_cast<f32>(v0));
                corners[2] =
                    to_world_offset(axis, depth, static_cast<f32>(u0) + W, static_cast<f32>(v0) + H);
                corners[3] = to_world_offset(axis, depth, static_cast<f32>(u0), static_cast<f32>(v0) + H);
            } else {
                corners[0] = to_world_offset(axis, depth, static_cast<f32>(u0), static_cast<f32>(v0));
                corners[1] = to_world_offset(axis, depth, static_cast<f32>(u0), static_cast<f32>(v0) + H);
                corners[2] =
                    to_world_offset(axis, depth, static_cast<f32>(u0) + W, static_cast<f32>(v0) + H);
                corners[3] = to_world_offset(axis, depth, static_cast<f32>(u0) + W, static_cast<f32>(v0));
            }

            u32 base_vertex = out.quad_count * 4;
            for (u32 i = 0; i < 4; i++) {
                glm::vec3 world = chunk_world_origin + corners[i] * VOXEL_SCALE;
                u32 packed = face_index | (ao[i] << 3u) |
                             (static_cast<u32>(material) << 8u);
                out.vertices[base_vertex + i] = {world.x, world.y, world.z,
                                                 packed};
            }

            u32 *idx = out.indices + out.quad_count * 6;
            idx[0] = base_vertex + 0;
            idx[1] = base_vertex + 1;
            idx[2] = base_vertex + 2;
            idx[3] = base_vertex + 0;
            idx[4] = base_vertex + 2;
            idx[5] = base_vertex + 3;

            out.quad_count++;
        }

        void greedy_merge_slice(FaceMask &mask, ChunkMeshData &out,
                                       glm::vec3 chunk_world_origin,
                                       const Chunk &chunk,
                                       const Chunk *neighbors[6], u32 axis,
                                       u32 positive, u32 depth) noexcept {
            for (u32 v = 0; v < CHUNK_SIZE; v++) {
                u32 bits = mask.rows[v];
                while (bits != 0) {
                    u32 u0 = static_cast<u32>(std::countr_zero(bits));
                    u32 shift = bits >> u0;
                    u32 width = static_cast<u32>(std::countr_zero(~shift));
                    u32 strip = static_cast<u32>(((1ull << width) - 1ull) << u0);

                    u32 height = 1;
                    while (v + height < CHUNK_SIZE &&
                           (mask.rows[v + height] & strip) == strip) {
                        mask.rows[v + height] &= ~strip;
                        height++;
                    }

                    bits &= ~strip;
                    mask.rows[v] = bits;

                    i32 na = static_cast<i32>(depth) - (positive ? 1 : 0);
                    u16 vox_val =
                        sample_voxel(chunk, neighbors, axis, na, u0, v);
                    u16 material = vox_val != 0 ? vox_val : static_cast<u16>(1);
                    emit_quad(out, chunk_world_origin, axis, positive, depth,
                              u0, v, width, height, material, chunk, neighbors);
                }
            }
        }

    } // anonymous namespace

    ChunkMeshData mesh_chunk(const Chunk &chunk, const Chunk *neighbors[6],
                             glm::ivec3 chunk_pos, ArenaAllocator &scratch) {
        constexpr u32 MAX_QUADS = 8192;

        ChunkMeshData out;
        out.vertices = scratch.push<MeshVertex>(MAX_QUADS * 4);
        out.indices = scratch.push<u32>(MAX_QUADS * 6);
        out.quad_count = 0;

        glm::vec3 chunk_world_origin =
            glm::vec3(chunk_pos) * static_cast<f32>(CHUNK_SIZE) * VOXEL_SCALE;

        auto *solid = scratch.push_zeroed<FaceMask>(CHUNK_SIZE);
        auto *face = scratch.push_zeroed<FaceMask>(CHUNK_SIZE);

        for (u32 axis = 0; axis < 3; axis++) {
            build_solid_masks(chunk, axis, solid);

            for (u32 positive = 0; positive <= 1; positive++) {
                build_face_masks(neighbors, axis, positive, solid, face);

                for (u32 a = 0; a < CHUNK_SIZE; a++) {
                    if (face[a].empty())
                        continue;
                    greedy_merge_slice(face[a], out, chunk_world_origin, chunk,
                                       neighbors, axis, positive, a);
                }
            }
        }

        return out;
    }

} // namespace mantle::bgm
