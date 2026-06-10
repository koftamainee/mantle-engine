// Copyright (c) 2026 Mantle. All rights reserved.

#pragma once

#include <glm/glm.hpp>
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

        void add_static_box(glm::vec3 pos, glm::vec3 half_extents);

      private:
        bool m_is_initialized = false;

        friend class CharacterController;

        struct Impl;
        Impl *m_impl;
    };

} // namespace mantle
