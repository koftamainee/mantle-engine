// Copyright (c) 2026 Mantle. All rights reserved.

#pragma once

#include <unordered_map>
#include <vector>

#include <glm/vec3.hpp>

#include "chunk.h"
#include "core/macros.h"
#include "core/memory/memory_block.h"
#include "core/memory/pmr/arena_resource.h"
#include "core/types.h"

namespace mantle {

    struct Vec3Hash {
        usize operator()(glm::ivec3 const &v) const noexcept {
            auto h = static_cast<usize>(v.x);
            h = h * 0x9e3779b9 + static_cast<usize>(v.y) + (h << 6) + (h >> 2);
            h = h * 0x9e3779b9 + static_cast<usize>(v.z) + (h << 6) + (h >> 2);
            return h;
        }
    };

    class ChunkStorageSystem final {
      public:
        MANTLE_DEFAULT_INIT(ChunkStorageSystem);

        void init(u32 capacity, MemoryBlock block);
        void destroy();

        u32  add_chunk(glm::ivec3 position);
        void remove_chunk(glm::ivec3 position);

        bool has_chunk(glm::ivec3 position) const;
        u32  get_index(glm::ivec3 position) const;

        Chunk       &get_chunk(u32 index);
        const Chunk &get_chunk(u32 index) const;
        Chunk       &get_chunk(glm::ivec3 position);
        const Chunk &get_chunk(glm::ivec3 position) const;

        glm::ivec3 get_position(u32 index) const;

        void mark_dirty(u32 index);
        void mark_clean(u32 index);
        void clear_dirty();

        bool is_dirty(u32 index) const;
        bool any_dirty() const;

        const std::pmr::vector<u32> &dirty_indices() const;

        u32 capacity() const { return m_capacity; }
        u32 count() const { return m_count; }

      private:
        struct Slot {
            Chunk      chunk = {};
            glm::ivec3 position = {};
            bool       dirty = false;
            bool       active = false;
        };

        u32 m_capacity = 0;
        u32 m_count = 0;

        ArenaAllocator                                     m_arena;
        ArenaResource                                      m_resource {};
        std::pmr::vector<Slot>                             m_slots;
        std::pmr::unordered_map<glm::ivec3, u32, Vec3Hash> m_map;
        std::pmr::vector<u32>                              m_dirty_queue;
        std::pmr::vector<u32>                              m_free_list;
        u32                                                m_next_free = 0;

        bool m_is_initialized = false;
    };
} // namespace mantle
