// Copyright (c) 2026 Mantle. All rights reserved.

#pragma once

#include <mutex>
#include <spdlog/logger.h>

#include "core/macros.h"
#include "core/memory/memory_block.h"
#include "core/memory/tlsf_allocator.h"

namespace mantle {

    class PhysicsSystem final {
      public:
        MANTLE_DEFAULT_INIT(PhysicsSystem);

        void init(MemoryBlock block);
        void update(f32 dt);
        void destroy();

      private:
        bool            m_is_initialized = false;
        TlsfAllocator   m_allocator {};
        std::mutex      m_alloc_mutex {};

        void   *m_jolt_temp_allocator  = nullptr;
        void   *m_jolt_job_system      = nullptr;
        void   *m_jolt_physics_system  = nullptr;
        void   *m_jolt_bp_interface    = nullptr;
        void   *m_jolt_object_vs_bp   = nullptr;
        void   *m_jolt_object_vs_object = nullptr;

        spdlog::logger *m_logger = nullptr;
    };

} // namespace mantle
