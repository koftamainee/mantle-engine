#pragma once

#include "core/memory/os_memory.h"
#include "core/memory/virtual_heap.h"
#include "core/types.h"

namespace mantle {

    class ArenaAllocator final {
      public:
        ArenaAllocator() = default;
        ~ArenaAllocator();

        ArenaAllocator(const ArenaAllocator &) = delete;
        ArenaAllocator &operator=(const ArenaAllocator &) = delete;
        ArenaAllocator(ArenaAllocator &&) = delete;
        ArenaAllocator &operator=(ArenaAllocator &&) = delete;

        void init(OSMemory &os, VirtualHeap &heap, usize size);
        void destroy();

        [[nodiscard]] void *push(usize size, usize align = 16);

        template <typename T>
        [[nodiscard]] T *push(usize count = 1) {
            return static_cast<T *>(push(sizeof(T) * count, alignof(T)));
        }

        struct Marker {
            usize offset;
        };

        Marker save() const;
        void restore(Marker marker);
        void reset();

        usize size() const;
        usize offset() const;

      private:
        OSMemory *m_os = nullptr;
        void *m_base = nullptr;
        usize m_size = 0;
        usize m_offset = 0;
        usize m_committed = 0;
        bool m_is_initialized = false;
    };

} // namespace mantle
