// Copyright (c) 2026 Mantle. All rights reserved.

#include "core/memory/arena_allocator.h"

#include "core/assert.h"

namespace mantle {

    ArenaAllocator::~ArenaAllocator() { destroy(); }

    void ArenaAllocator::init(MemoryBlock block, std::string_view debug_name) {
        MANTLE_CHECK(!m_is_initialized);
        MANTLE_CHECK(block.ptr != nullptr);
        MANTLE_CHECK(block.size > 0);

        m_base = block.ptr;
        m_size = block.size;
        m_offset = 0;
        m_is_initialized = true;
    }

    void ArenaAllocator::destroy() {
        if (m_is_initialized) {
            m_base = nullptr;
            m_size = 0;
            m_offset = 0;
            m_is_initialized = false;
        }
    }

    void *ArenaAllocator::push(usize size, usize align) {
        MANTLE_CHECK(m_is_initialized);

        usize aligned_offset = (m_offset + (align - 1)) & ~(align - 1);
        usize new_offset = aligned_offset + size;

        MANTLE_FATAL(new_offset > m_size, "Out of memory");

        void *ptr = static_cast<u8 *>(m_base) + aligned_offset;
        m_offset = new_offset;
        return ptr;
    }

    ArenaAllocator::Marker ArenaAllocator::save() const {
        MANTLE_CHECK(m_is_initialized);
        return {m_offset};
    }

    void ArenaAllocator::restore(Marker marker) {
        MANTLE_CHECK(m_is_initialized);
        MANTLE_CHECK(marker.offset <= m_offset);
        m_offset = marker.offset;
    }

    void ArenaAllocator::reset() {
        MANTLE_CHECK(m_is_initialized);
        m_offset = 0;
    }

    usize ArenaAllocator::size() const {
        MANTLE_CHECK(m_is_initialized);
        return m_size;
    }

    usize ArenaAllocator::offset() const {
        MANTLE_CHECK(m_is_initialized);
        return m_offset;
    }

    usize ArenaAllocator::remaining() const {
        MANTLE_CHECK(m_is_initialized);
        return m_size - m_offset;
    }

} // namespace mantle
