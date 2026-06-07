#include "world/chunk_storage_system.h"

#include "core/assert.h"
#include "core/memory/memory_units.h"
#include "core/memory/virtual_heap.h"

namespace mantle {
    ChunkStorageSystem::~ChunkStorageSystem() { destroy(); }

    void ChunkStorageSystem::init(u32 capacity, VirtualHeap *heap) {
        MANTLE_CHECK(!m_is_initialized);
        MANTLE_CHECK(capacity > 0);
        MANTLE_CHECK(heap != nullptr);

        usize arena_size = capacity * sizeof(Slot) + kilobytes(64);
        m_arena.init(heap->take(arena_size));
        m_resource = ArenaResource(&m_arena);

        m_capacity = capacity;
        m_count = 0;

        m_slots = std::pmr::vector<Slot>(&m_resource);
        m_slots.resize(capacity);

        m_map = std::pmr::unordered_map<glm::ivec3, u32, Vec3Hash>(&m_resource);
        m_dirty_queue = std::pmr::vector<u32>(&m_resource);
        m_free_list = std::pmr::vector<u32>(&m_resource);
        m_next_free = 0;

        m_is_initialized = true;
    }

    void ChunkStorageSystem::destroy() {
        if (m_is_initialized) {
            m_free_list.clear();
            m_dirty_queue.clear();
            m_map.clear();
            m_slots.clear();
            m_arena.destroy();
            m_capacity = 0;
            m_count = 0;
            m_is_initialized = false;
        }
    }

    u32 ChunkStorageSystem::add_chunk(glm::ivec3 position) {
        MANTLE_CHECK(m_is_initialized);
        MANTLE_CHECK(m_count < m_capacity);

        if (auto it = m_map.find(position); it != m_map.end()) {
            return it->second;
        }

        u32 index;
        if (!m_free_list.empty()) {
            index = m_free_list.back();
            m_free_list.pop_back();
        } else {
            index = m_next_free++;
        }

        m_slots[index].chunk = {};
        m_slots[index].position = position;
        m_slots[index].dirty = true;
        m_slots[index].active = true;
        m_map[position] = index;
        m_dirty_queue.push_back(index);
        m_count++;
        return index;
    }

    void ChunkStorageSystem::remove_chunk(glm::ivec3 position) {
        MANTLE_CHECK(m_is_initialized);

        auto it = m_map.find(position);
        MANTLE_CHECK(it != m_map.end());

        u32 index = it->second;
        m_slots[index].active = false;
        m_slots[index].dirty = false;
        m_map.erase(it);
        m_free_list.push_back(index);
        m_count--;
    }

    bool ChunkStorageSystem::has_chunk(glm::ivec3 position) const {
        MANTLE_CHECK(m_is_initialized);
        return m_map.contains(position);
    }

    u32 ChunkStorageSystem::get_index(glm::ivec3 position) const {
        MANTLE_CHECK(m_is_initialized);
        return m_map.at(position);
    }

    Chunk &ChunkStorageSystem::get_chunk(u32 index) {
        MANTLE_CHECK(m_is_initialized);
        MANTLE_CHECK(index < m_capacity);
        MANTLE_CHECK(m_slots[index].active);
        return m_slots[index].chunk;
    }

    const Chunk &ChunkStorageSystem::get_chunk(u32 index) const {
        MANTLE_CHECK(m_is_initialized);
        MANTLE_CHECK(index < m_capacity);
        MANTLE_CHECK(m_slots[index].active);
        return m_slots[index].chunk;
    }

    Chunk &ChunkStorageSystem::get_chunk(glm::ivec3 position) {
        MANTLE_CHECK(m_is_initialized);
        u32 index = m_map.at(position);
        return m_slots[index].chunk;
    }

    const Chunk &ChunkStorageSystem::get_chunk(glm::ivec3 position) const {
        MANTLE_CHECK(m_is_initialized);
        u32 index = m_map.at(position);
        return m_slots[index].chunk;
    }

    glm::ivec3 ChunkStorageSystem::get_position(u32 index) const {
        MANTLE_CHECK(m_is_initialized);
        MANTLE_CHECK(index < m_capacity);
        MANTLE_CHECK(m_slots[index].active);
        return m_slots[index].position;
    }

    bool ChunkStorageSystem::is_dirty(u32 index) const {
        MANTLE_CHECK(m_is_initialized);
        MANTLE_CHECK(index < m_capacity);
        return m_slots[index].active && m_slots[index].dirty;
    }

    void ChunkStorageSystem::mark_dirty(u32 index) {
        MANTLE_CHECK(m_is_initialized);
        MANTLE_CHECK(index < m_capacity);
        if (!m_slots[index].dirty) {
            m_slots[index].dirty = true;
            m_dirty_queue.push_back(index);
        }
    }

    void ChunkStorageSystem::mark_clean(u32 index) {
        MANTLE_CHECK(m_is_initialized);
        MANTLE_CHECK(index < m_capacity);
        m_slots[index].dirty = false;
    }

    bool ChunkStorageSystem::any_dirty() const {
        MANTLE_CHECK(m_is_initialized);
        for (u32 i = 0; i < m_capacity; i++) {
            if (m_slots[i].active && m_slots[i].dirty) return true;
        }
        return false;
    }

    void ChunkStorageSystem::clear_dirty() {
        MANTLE_CHECK(m_is_initialized);
        for (u32 i : m_dirty_queue) {
            m_slots[i].dirty = false;
        }
        m_dirty_queue.clear();
    }

    const std::pmr::vector<u32> &ChunkStorageSystem::dirty_indices() const {
        MANTLE_CHECK(m_is_initialized);
        return m_dirty_queue;
    }

} // namespace mantle
