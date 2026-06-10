// Copyright (c) 2026 Mantle. All rights reserved.

#pragma once

// fix-includes off
#include <Jolt/Jolt.h>
// fix-includes on

#include <Jolt/Core/TempAllocator.h>

#include "core/macros.h"
#include "core/memory/arena_allocator.h"
#include "core/memory/memory_block.h"
#include "core/memory/thread_safe_allocator.h"

namespace mantle {
    class ArenaTempAllocator final : public JPH::TempAllocator {
      public:
        MANTLE_DEFAULT_INIT(ArenaTempAllocator);

        void init(MemoryBlock mem, std::string_view debug_name = {});
        void destroy();

        void *Allocate(JPH::uint inSize) override;
        void  Free(void *inAddress, JPH::uint inSize) override;

      private:
        bool                                m_is_initialized = false;
        ThreadSafeAllocator<ArenaAllocator> m_allocator {};
    };
} // namespace mantle
