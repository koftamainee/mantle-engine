// Copyright (c) 2026 Mantle. All rights reserved.

#pragma once

#include <cstring>
#include <string_view>
#include <type_traits>
#include <utility>

#include "mantle/core/assert.h"
#include "mantle/core/macros.h"
#include "mantle/core/memory/memory_block.h"
#include "mantle/core/types.h"

namespace mantle {

    class ArenaAllocator final {
      public:
        ArenaAllocator() = default;
        ~ArenaAllocator();

        MANTLE_NO_COPY(ArenaAllocator);
        ArenaAllocator(ArenaAllocator &&) = default;

        void init(MemoryBlock block, std::string_view debug_name = {});
        void destroy();

        [[nodiscard]] void *alloc(usize size, usize align = alignof(std::max_align_t)) {
            return push(size, align);
        }
        [[nodiscard]] void *push(usize size, usize align = alignof(std::max_align_t));

        template <typename T>
        [[nodiscard]] T *alloc(usize count = 1) {
            return push<T>(count);
        }

        template <typename T>
        [[nodiscard]] T *push(usize count = 1) {
            static_assert(std::is_trivially_constructible_v<T>);
            return static_cast<T *>(push(sizeof(T) * count, alignof(T)));
        }

        template <typename T>
        [[nodiscard]] T *alloc_zeroed(usize count = 1) {
            return push_zeroed<T>(count);
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

        // NOTE: use this with care. free calls should be in LIFO order for this to not UB
        void free(void *ptr);

        void *realloc(void *ptr, usize size) {
            MANTLE_CHECKF(false, "Called realloc in arena allocator");
            return nullptr;
        }

        struct Marker {
            usize offset;
        };

        Marker save() const;
        void   restore(Marker marker);
        void   reset();

        usize size() const;
        usize offset() const;
        usize remaining() const;

      private:
        void *m_base = nullptr;
        usize m_size = 0;
        usize m_offset = 0;
        bool  m_is_initialized = false;
    };

} // namespace mantle
