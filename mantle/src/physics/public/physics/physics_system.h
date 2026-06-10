// Copyright (c) 2026 Mantle. All rights reserved.

#pragma once

#include <mutex>
#include <spdlog/logger.h>

#include "core/macros.h"
#include "core/memory/memory_block.h"
#include "core/memory/thread_safe_allocator.h"
#include "core/memory/tlsf_allocator.h"

namespace mantle {

    class PhysicsSystem final {
      public:
        MANTLE_DEFAULT_INIT(PhysicsSystem);

        void init(MemoryBlock block);
        void update(f32 dt);
        void destroy();

      private:
        bool                               m_is_initialized = false;
        ThreadSafeAllocator<TlsfAllocator> m_allocator {};
        spdlog::logger                    *m_logger = nullptr;
    };

} // namespace mantle
