// Copyright (c) 2026 Mantle. All rights reserved.

#pragma once

#include <string_view>

#include <glm/glm.hpp>
#include <spdlog/spdlog.h>

#include <flecs.h>

#include "mantle/core/macros.h"
#include "mantle/core/types.h"

#include "mantle/ecs/entity.h"

namespace mantle {
    class Ecs final {
      public:
        MANTLE_DEFAULT_INIT(Ecs);

        void init();
        void destroy();
        void update(f32 dt);

        Entity create_entity(std::string_view name = {});
        Entity lookup(std::string_view name) const;

        template<typename T>
        const T *get_singleton() const {
            return m_world.has<T>() ? &m_world.get<T>() : nullptr;
        }

        template<typename T>
        void set_singleton(T &&value) {
            m_world.set<T>(std::forward<T>(value));
        }

        template<typename T>
        bool has_singleton() const {
            return m_world.has<T>();
        }

        template<typename T>
        T &ensure_singleton() {
            return m_world.ensure<T>();
        }

        template<typename T, typename... Args>
        T &emplace_singleton(Args &&... args) {
            T &ref = m_world.ensure<T>();
            ref = T(std::forward<Args>(args)...);
            return ref;
        }

        template<typename... Components, typename F>
        void foreach(F &&callback) const {
            auto q = m_world.query<Components...>();
            q.each(std::forward<F>(callback));
        }

      private:
        bool            m_is_initialized = false;
        spdlog::logger *m_logger = nullptr;

        flecs::world m_world{};
    };
} // namespace mantle
