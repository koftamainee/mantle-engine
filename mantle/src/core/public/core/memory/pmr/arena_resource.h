#pragma once

#include <memory_resource>

#include "../arena_allocator.h"
#include "core/assert.h"

namespace mantle {
    class ArenaResource final : public std::pmr::memory_resource {
      public:
        ArenaResource() : m_arena(nullptr) {}
        explicit ArenaResource(ArenaAllocator *arena) : m_arena(arena) {}

      private:
        void *do_allocate(usize size, usize align) override {
            MANTLE_CHECK(m_arena != nullptr);
            return m_arena->push(size, align);
        }

        void do_deallocate(void *memory, usize size, usize align) override {}

        bool do_is_equal(const memory_resource &other) const noexcept override {
            return this == &other;
        }

      private:
        ArenaAllocator *m_arena;
    };
} // namespace mantle
