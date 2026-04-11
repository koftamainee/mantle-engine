#pragma once
#include "arena_allocator.h"

namespace mantle {
    struct ScopeArena final {
        ArenaAllocator *arena;
        ArenaAllocator::Marker tag;
        explicit ScopeArena(ArenaAllocator *a) : arena(a), tag(a->save()) {}
        ~ScopeArena() { arena->restore(tag); }
    };
} // namespace mantle
