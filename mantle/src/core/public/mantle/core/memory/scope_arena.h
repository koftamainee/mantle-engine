// Copyright (c) 2026 Mantle. All rights reserved.

#pragma once

#include "arena_allocator.h"

namespace mantle {
    struct ScopeArena final {
        ArenaAllocator        *arena;
        ArenaAllocator::Marker tag;

        // NOTE: maybe use only no copy, and implement move operator
        MANTLE_NO_COPY_NO_MOVE(ScopeArena);

        explicit ScopeArena(ArenaAllocator *a) : arena(a), tag(a->save()) {}
        ~ScopeArena() { arena->restore(tag); }
    };
} // namespace mantle
