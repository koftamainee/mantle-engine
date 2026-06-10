// Copyright (c) 2026 Mantle. All rights reserved.

#pragma once

#include <spdlog/logger.h>

#include "core/macros.h"
#include "core/memory/memory_block.h"
#include "core/memory/thread_safe_allocator.h"
#include "core/memory/tlsf_allocator.h"
#include "arena_temp_allocator.h"
#include "Jolt/Core/JobSystemThreadPool.h"

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

        ArenaTempAllocator m_temp_allocator{};

        // Use heap allocation to init it in init(), not constructor
        JPH::JobSystemThreadPool *m_job_system = nullptr;

        constexpr static u32 kMaxBodies = 1024;
        constexpr static u32 kNumBodyMutexes = 0;
        constexpr static u32 kMaxBodyPairs = 1024;
        constexpr static u32 kMaxContactConstraints = 1024;
    };

} // namespace mantle
