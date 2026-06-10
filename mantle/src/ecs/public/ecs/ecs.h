// Copyright (c) 2026 Mantle. All rights reserved.

#pragma once

#include <glm/glm.hpp>
#include <spdlog/spdlog.h>

#include <flecs.h>

#include "core/macros.h"
#include "core/types.h"

namespace mantle {
    class CharacterController;
    class Window;

    class Ecs final {
      public:
        MANTLE_DEFAULT_INIT(Ecs);

        void init(Window &window, f32 camera_aspect, CharacterController &character);
        void destroy();
        void update(f32 delta_time);

        glm::mat4 camera_view_proj() const;
        void      set_camera_aspect(f32 aspect);

      private:
        bool            m_is_initialized = false;
        spdlog::logger *m_logger = nullptr;

        flecs::world m_world {};
    };
} // namespace mantle
