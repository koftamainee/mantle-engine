#include "world/chunk_meshing_system.h"
#include "world/chunk_rendering_system.h"
#include "world/chunk_storage_system.h"

#include <cstring>
#include <vector>

#include "bgm.h"
#include "chunk_helpers.h"
#include "core/assert.h"
#include "core/memory/scope_arena.h"
#include "renderer/gpu_resource_manager.h"
#include "renderer/renderer.h"
#include "spdlog/spdlog.h"

namespace mantle {

    ChunkMeshingSystem::~ChunkMeshingSystem() { destroy(); }

    void ChunkMeshingSystem::init() {
        MANTLE_CHECK(!m_is_initialized);
        m_logger = spdlog::get("world").get();
        m_is_initialized = true;
        m_logger->info("Chunk meshing system initialized");
    }

    void ChunkMeshingSystem::destroy() {
        m_is_initialized = false;
    }

    namespace {

        struct MeshTaskData {
            const ChunkStorageSystem *storage;
            u32 idx;
            MeshVertex *vertex_out;
            u32 *index_out;
            u32 max_quads;
            u32 quad_count;
        };

        void mesh_work(ArenaAllocator &scratch, void *data) {
            auto *d = static_cast<MeshTaskData *>(data);
            const Chunk &chunk = d->storage->get_chunk(d->idx);
            glm::ivec3 pos = d->storage->get_position(d->idx);

            const Chunk *neighbors[6];
            get_neighbors(*d->storage, pos, neighbors);

            ChunkMeshData mesh =
                bgm::mesh_chunk(chunk, neighbors, pos, scratch);

            u32 q = mesh.quad_count;
            if (q > d->max_quads) {
                q = d->max_quads;
            }
            d->quad_count = q;

            if (q > 0) {
                std::memcpy(d->vertex_out, mesh.vertices, mesh.vertex_bytes());
                std::memcpy(d->index_out, mesh.indices, mesh.index_bytes());
            }

            scratch.reset();
        }

    } // namespace

    void ChunkMeshingSystem::upload_dirty(Renderer &renderer,
                                          ChunkStorageSystem &storage,
                                          ArenaAllocator &scratch,
                                          WorkerPool *pool,
                                          ChunkRenderingSystem &rendering) const {
        MANTLE_CHECK(m_is_initialized);

        if (!storage.any_dirty())
            return;

        auto &rm = renderer.resource_manager();
        const auto &dirty = storage.dirty_indices();

        BufferHandle vertex_buffer = rendering.vertex_buffer();
        BufferHandle index_buffer = rendering.index_buffer();
        ChunkMeshSlot *slots = rendering.slots();

        if (pool) {
            std::vector<MeshTaskData> tasks;
            tasks.reserve(dirty.size());

            u32 count = 0;
            for (u32 i : dirty) {
                if (storage.is_dirty(i))
                    count++;
            }

            usize vert_stride = MAX_QUADS_PER_CHUNK * 4 * sizeof(MeshVertex);
            usize idx_stride = MAX_QUADS_PER_CHUNK * 6 * sizeof(u32);
            void *vert_block = scratch.push(vert_stride * count, 16);
            void *idx_block = scratch.push(idx_stride * count, 16);

            u32 slot = 0;
            for (u32 i : dirty) {
                if (!storage.is_dirty(i))
                    continue;
                tasks.push_back({
                    .storage = &storage,
                    .idx = i,
                    .vertex_out = static_cast<MeshVertex *>(vert_block) +
                        slot * MAX_QUADS_PER_CHUNK * 4,
                    .index_out =
                        static_cast<u32 *>(idx_block) + slot * MAX_QUADS_PER_CHUNK * 6,
                    .max_quads = MAX_QUADS_PER_CHUNK,
                    .quad_count = 0,
                });
                pool->submit(mesh_work, &tasks.back());
                slot++;
            }
            pool->wait();

            for (auto &t : tasks) {
                u32 q = t.quad_count;
                glm::ivec3 pos = t.storage->get_position(t.idx);
                auto &mesh_slot = slots[t.idx];
                mesh_slot.position_x = pos.x;
                mesh_slot.position_y = pos.y;
                mesh_slot.position_z = pos.z;
                mesh_slot.quad_count = q;

                if (q > 0) {
                    rm.update_buffer(vertex_buffer, t.vertex_out,
                                     static_cast<usize>(q) * 4 * sizeof(MeshVertex),
                                     mesh_slot.vertex_offset);
                    rm.update_buffer(index_buffer, t.index_out,
                                     static_cast<usize>(q) * 6 * sizeof(u32),
                                     mesh_slot.index_offset);
                }

                storage.mark_clean(t.idx);
            }
        } else {
            for (u32 i : dirty) {
                ScopeArena scope(&scratch);
                if (!storage.is_dirty(i)) {
                    continue;
                }

                const Chunk &chunk = storage.get_chunk(i);
                glm::ivec3 pos = storage.get_position(i);

                const Chunk *neighbors[6];
                get_neighbors(storage, pos, neighbors);

                ChunkMeshData mesh =
                    bgm::mesh_chunk(chunk, neighbors, pos, scratch);

                u32 q = mesh.quad_count;
                if (q > MAX_QUADS_PER_CHUNK) {
                    m_logger->warn("Chunk mesh truncated: {} → {} quads", q,
                                 MAX_QUADS_PER_CHUNK);
                    q = MAX_QUADS_PER_CHUNK;
                }

                auto &slot = slots[i];
                slot.position_x = pos.x;
                slot.position_y = pos.y;
                slot.position_z = pos.z;
                slot.quad_count = q;

                if (q > 0) {
                    rm.update_buffer(vertex_buffer, mesh.vertices,
                                     (usize)q * 4 * sizeof(MeshVertex),
                                     slot.vertex_offset);
                    rm.update_buffer(index_buffer, mesh.indices,
                                     (usize)q * 6 * sizeof(u32),
                                     slot.index_offset);
                }

                storage.mark_clean(i);
            }
        }

        storage.clear_dirty();
    }

} // namespace mantle
