// Copyright (c) 2026 Mantle. All rights reserved.

#pragma once

#include <glm/glm.hpp>

#include "mantle/core/macros.h"
#include "mantle/core/types.h"

namespace mantle {

    class PhysicsSystem;

    class CharacterController final {
      public:
        MANTLE_DEFAULT_INIT(CharacterController);

        void init(PhysicsSystem &physics, glm::vec3 start_pos);
        void destroy();

        void      move(glm::vec3 velocity, f32 dt);
        glm::vec3 get_position() const;
        bool      is_grounded() const;

        glm::vec3 gravity = {0, -9.81f, 0};

      private:
        bool m_is_initialized = false;

        struct Impl;
        Impl *m_impl = nullptr;
    };

} // namespace mantle
