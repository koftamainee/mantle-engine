// Copyright (c) 2026 Mantle. All rights reserved.

#include "arena_temp_allocator.h"

namespace mantle {
    void ArenaTempAllocator::init(MemoryBlock mem, std::string_view debug_name) {
        MANTLE_CHECK(!m_is_initialized);
        m_allocator.init(mem, debug_name);
        m_is_initialized = true;
    }

    void ArenaTempAllocator::destroy() {
        if (m_is_initialized) {
            m_allocator.destroy();
            m_is_initialized = false;
        }
    }

    void *ArenaTempAllocator::Allocate(JPH::uint inSize) {
        MANTLE_CHECK(m_is_initialized);
        return m_allocator.alloc(inSize);
    }

    void ArenaTempAllocator::Free(void *inAddress, JPH::uint inSize) {
        MANTLE_CHECK(m_is_initialized);
        m_allocator.free(inAddress);
    }

    void ArenaTempAllocator::Reset() { m_allocator.reset(); }
} // namespace mantle
