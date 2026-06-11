// Copyright (c) 2026 Mantle. All rights reserved.

#pragma once

#include <string_view>

#include <flecs.h>

#include "mantle/core/types.h"

namespace mantle {

    class Entity {
        friend class Ecs;

      public:
        Entity() = default;

        template<typename T, typename... Args>
        Entity &add(Args &&... args) {
            m_entity.set<T>(T(std::forward<Args>(args)...));
            return *this;
        }

        template<typename T>
        Entity &set(T &&value) {
            m_entity.set<T>(std::forward<T>(value));
            return *this;
        }

        template<typename T>
        T *get_mut() {
            return m_entity.is_alive() ? &m_entity.ensure<T>() : nullptr;
        }

        template<typename T>
        const T *get() const {
            return m_entity.get<T>();
        }

        template<typename T>
        bool has() const {
            return m_entity.has<T>();
        }

        Entity &set_name(const char *name) {
            m_entity.set_name(name);
            return *this;
        }

        const char *name() const {
            return m_entity.name();
        }

        bool is_alive() const {
            return m_entity.is_alive();
        }

        void destroy() {
            m_entity.destruct();
        }

        bool operator==(const Entity &other) const {
            return m_entity == other.m_entity;
        }

        bool operator!=(const Entity &other) const {
            return m_entity != other.m_entity;
        }

    private:
        friend class Engine;
        explicit Entity(flecs::entity e) : m_entity(e) {}

        flecs::entity m_entity;
    };

} // namespace mantle
