// Copyright (c) 2026 Mantle. All rights reserved.

#pragma once

#include <cstring>
#include <string_view>
#include <utility>

#include "mantle/core/macros.h"
#include "mantle/core/memory/memory_block.h"
#include "mantle/core/types.h"
#include "tlsf.h"

namespace mantle {

    class TlsfAllocator final {
      public:
        TlsfAllocator() = default;
        ~TlsfAllocator();

        MANTLE_NO_COPY(TlsfAllocator);

        void init(MemoryBlock block, std::string_view debug_name = {});
        void destroy();

        [[nodiscard]] void *alloc(usize size, usize align = alignof(std::max_align_t));
        [[nodiscard]] void *realloc(void *ptr, usize size);
        void                free(void *ptr);

        template <typename T>
        [[nodiscard]] T *alloc(usize count = 1) {
            return static_cast<T *>(alloc(sizeof(T) * count, alignof(T)));
        }

        template <typename T>
        [[nodiscard]] T *alloc_zeroed(usize count = 1) {
            T *ptr = static_cast<T *>(alloc(sizeof(T) * count, alignof(T)));
            std::memset(ptr, 0, sizeof(T) * count);
            return ptr;
        }

        template <typename T, typename... Args>
        [[nodiscard]] T *emplace(Args &&...args) {
            void *mem = alloc(sizeof(T), alignof(T));
            return new (mem) T(std::forward<Args>(args)...);
        }

      private:
        tlsf_t m_tlsf = nullptr;
        bool   m_is_initialized = false;
    };

} // namespace mantle
