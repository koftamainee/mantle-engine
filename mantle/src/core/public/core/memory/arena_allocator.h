#pragma once

#include <cstring>
#include <type_traits>
#include <utility>

#include "core/macros.h"
#include "core/memory/memory_block.h"
#include "core/types.h"

namespace mantle {

    class ArenaAllocator final {
      public:
        ArenaAllocator() = default;
        ~ArenaAllocator();

        MANTLE_NO_COPY_NO_MOVE(ArenaAllocator);

        void init(MemoryBlock block);
        void destroy();

        [[nodiscard]] void *push(usize size,
                                 usize align = alignof(std::max_align_t));

        template <typename T>
        [[nodiscard]] T *push(usize count = 1) {
            static_assert(std::is_trivially_constructible_v<T>);
            return static_cast<T *>(push(sizeof(T) * count, alignof(T)));
        }

        template <typename T>
        [[nodiscard]] T *push_zeroed(usize count = 1) {
            static_assert(std::is_trivially_constructible_v<T>);
            T *ptr = static_cast<T *>(push(sizeof(T) * count, alignof(T)));
            std::memset(ptr, 0, sizeof(T) * count);
            return ptr;
        }

        template <typename T, typename... Args>
        [[nodiscard]] T *emplace(Args &&...args) {
            void *mem = push(sizeof(T), alignof(T));
            return new (mem) T(std::forward<Args>(args)...);
        }

        struct Marker {
            usize offset;
        };

        Marker save() const;
        void restore(Marker marker);
        void reset();

        usize size() const;
        usize offset() const;
        usize remaining() const;

      private:
        void *m_base = nullptr;
        usize m_size = 0;
        usize m_offset = 0;
        bool m_is_initialized = false;
    };

} // namespace mantle
