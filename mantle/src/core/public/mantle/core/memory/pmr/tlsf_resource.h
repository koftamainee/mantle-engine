// Copyright (c) 2026 Mantle. All rights reserved.

#pragma once

#include <memory_resource>

#include "mantle/core/assert.h"
#include "mantle/core/memory/tlsf_allocator.h"

namespace mantle {

    class TlsfResource final : public std::pmr::memory_resource {
      public:
        TlsfResource() : m_allocator(nullptr) {}
        explicit TlsfResource(TlsfAllocator *allocator) : m_allocator(allocator) {}

      private:
        void *do_allocate(usize size, usize align) override {
            MANTLE_CHECK(m_allocator != nullptr);
            return m_allocator->alloc(size, align);
        }

        void do_deallocate(void *memory, usize size, usize align) override {
            MANTLE_CHECK(m_allocator != nullptr);
            m_allocator->free(memory);
        }

        bool do_is_equal(const memory_resource &other) const noexcept override {
            return this == &other;
        }

      private:
        TlsfAllocator *m_allocator;
    };

} // namespace mantle
