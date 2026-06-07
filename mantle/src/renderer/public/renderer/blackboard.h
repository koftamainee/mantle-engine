#pragma once
#include "core/assert.h"
#include "core/macros.h"
#include "core/types.h"

#include <memory_resource>
#include <unordered_map>

namespace mantle {

    class Blackboard final {
      public:
        struct TypeIdBase {
          protected:
            static inline u32 s_next = 0;
        };

        template <typename T>
        struct TypeId : TypeIdBase {
            static u32 id() {
                static u32 s_id = s_next++;
                return s_id;
            }
        };

        explicit Blackboard(std::pmr::memory_resource *memory = nullptr)
            : m_entries(memory) {}

        MANTLE_NO_COPY_NO_MOVE(Blackboard);

        template <typename T>
        T &add(T value) {
            u32 tid = TypeId<T>::id();
            MANTLE_CHECKF(!m_entries.contains(tid),
                   "Blackboard::add(): type already added. "
                   "Keep the T& from add() to modify instead of adding again.");
            auto *ptr = m_entries.get_allocator()
                            .resource()
                            ->allocate(sizeof(T), alignof(T));
            new (ptr) T(std::move(value));
            m_entries[tid] = ptr;
            return *static_cast<T *>(ptr);
        }

        template <typename T>
            requires std::is_default_constructible_v<T>
        T &add() {
            u32 tid = TypeId<T>::id();
            MANTLE_CHECKF(!m_entries.contains(tid),
                   "Blackboard::add(): type already added. "
                   "Keep the T& from add() to modify instead of adding again.");
            auto *ptr = m_entries.get_allocator()
                            .resource()
                            ->allocate(sizeof(T), alignof(T));
            new (ptr) T{};
            m_entries[tid] = ptr;
            return *static_cast<T *>(ptr);
        }

        template <typename T>
        const T &get() const {
            u32 tid = TypeId<T>::id();
            auto it = m_entries.find(tid);
            MANTLE_CHECKF(it != m_entries.end(),
                   "Blackboard::get(): type not found. "
                   "Call add<T>() before get<T>(). "
                   "Check that the producing module's add_passes() "
                   "runs before this one.");
            return *static_cast<const T *>(it->second);
        }

        template <typename T>
        bool has() const {
            return m_entries.contains(TypeId<T>::id());
        }

      private:
        std::pmr::unordered_map<u32, void *> m_entries;
    };

} // namespace mantle
