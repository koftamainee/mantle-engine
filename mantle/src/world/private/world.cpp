#include "world/world.h"

#include <new>

#include "core/assert.h"

namespace mantle {
    World::~World() { destroy(); }

    void World::init() {
        check(!m_is_initialized);
        m_chunk_pool.clear();
        m_chunk_index.clear();

        m_chunk_pool.reserve(4096); // this is kinda bad i need to fix it
        m_is_initialized = true;
        spdlog::info("World is initialized");
    }

    void World::destroy() {
        if (m_is_initialized) {
            m_chunk_pool.clear();
            m_chunk_index.clear();

            m_is_initialized = false;
            spdlog::info("World is destroyed");
        }
    }

    Chunk &World::get_or_create_chunk(glm::ivec3 chunk_coords) {
        check(m_is_initialized);
        Chunk *chunk = get_chunk(chunk_coords);
        if (chunk == nullptr) {
            chunk = generate_chunk(chunk_coords);
        }

        // generate chunk returns nullptr only for already existing chunks
        return *chunk;
    }

    Chunk *World::get_chunk(glm::ivec3 chunk_coords) {
        check(m_is_initialized);
        auto it = m_chunk_index.find(chunk_coords);
        if (it == m_chunk_index.end()) {
            return nullptr;
        }
        return &m_chunk_pool[it->second];
    }

    Chunk *World::generate_chunk(glm::ivec3 chunk_coords) {
        check(m_is_initialized);
        auto it = m_chunk_index.find(chunk_coords);
        if (it != m_chunk_index.end()) {
            return nullptr; // no regenerating chunks for now
        }

        u32 index = m_chunk_pool.size();
        Chunk &chunk = m_chunk_pool.emplace_back(chunk_coords);
        m_chunk_index.emplace(chunk_coords, index);

        Chunk::VoxelSpan voxels = chunk.voxels();

        for (u32 x = 0; x < Chunk::s_chunk_size; x++) {
            for (u32 y = 0; y < Chunk::s_chunk_size; y++) {
                for (u32 z = 0; z < Chunk::s_chunk_size; z++) {
                    i32 world_y = static_cast<i32>(chunk_coords.y * Chunk::s_chunk_size + y);
                    if (rand() % 100 > 90) {
                        voxels[x, y, z].id = world_y > 0 ? 0u : 1u;
                    } else {
                        voxels[x, y, z].id = world_y = 0u;
                    }
                }
            }
        }

        return &chunk;
    }

    void World::for_each_chunk(const std::function<void(Chunk &)>& fn) {
        check(m_is_initialized);
        for (auto &chunk : m_chunk_pool) {
            fn(chunk);
        }
    }
} // namespace mantle
