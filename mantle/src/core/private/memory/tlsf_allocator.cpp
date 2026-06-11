// Copyright (c) 2026 Mantle. All rights reserved.

#include "mantle/core/memory/tlsf_allocator.h"

#include <tlsf.h>

#include "mantle/core/assert.h"

namespace mantle {

    TlsfAllocator::~TlsfAllocator() { destroy(); }

    void TlsfAllocator::init(MemoryBlock block, std::string_view /*debug_name*/) {
        MANTLE_CHECK(!m_is_initialized);
        MANTLE_CHECK(block.ptr != nullptr);
        MANTLE_CHECK(block.size > 0);

        m_tlsf = tlsf_create_with_pool(block.ptr, block.size);
        MANTLE_FATAL(m_tlsf == nullptr, "Out of memory");
        m_is_initialized = true;
    }

    void TlsfAllocator::destroy() {
        if (m_is_initialized) {
            tlsf_destroy(m_tlsf);
            m_tlsf = nullptr;
            m_is_initialized = false;
        }
    }

    void *TlsfAllocator::alloc(usize size, usize align) {
        MANTLE_CHECK(m_is_initialized);
        MANTLE_CHECK(size > 0);
        MANTLE_CHECK(align > 0);
        void *ptr = tlsf_memalign(m_tlsf, align, size);
        MANTLE_FATAL(ptr == nullptr, "Out of memory");
        return ptr;
    }

    void *TlsfAllocator::realloc(void *ptr, usize size) {
        MANTLE_CHECK(m_is_initialized);
        MANTLE_CHECK(size > 0);

        void *result = tlsf_realloc(m_tlsf, ptr, size);
        MANTLE_FATAL(result == nullptr, "Out of memory");
        return result;
    }

    void TlsfAllocator::free(void *ptr) {
        MANTLE_CHECK(m_is_initialized);
        if (!ptr) {
            return;
        }

        tlsf_free(m_tlsf, ptr);
    }

} // namespace mantle
