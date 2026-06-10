// Copyright (c) 2026 Mantle. All rights reserved.

#pragma once

#include <spdlog/logger.h>

#include "core/macros.h"
#include "core/memory/memory_block.h"

namespace mantle {

    class PhysicsSystem final {
      public:
        MANTLE_DEFAULT_INIT(PhysicsSystem);

        void init(MemoryBlock block);
        void update(f32 dt);
        void destroy();

      private:
        bool m_is_initialized = false;

        struct Impl;
        Impl *m_impl;
    };

} // namespace mantle
