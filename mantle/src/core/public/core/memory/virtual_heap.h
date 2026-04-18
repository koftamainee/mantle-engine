#pragma once

#include "core/macros.h"
#include "core/memory/memory_block.h"
#include "core/memory/os_memory.h"
#include "core/types.h"

namespace mantle {

    class VirtualHeap final {
      public:
        VirtualHeap() = default;
        ~VirtualHeap();

        MANTLE_NO_COPY_NO_MOVE(VirtualHeap);

        void init(OSMemory &os, usize reserve_size);
        void destroy();

        [[nodiscard]] MemoryBlock take(usize size);

        usize reserved() const;
        usize used() const;
        usize committed() const;

      private:
        OSMemory *m_os = nullptr;
        void *m_base = nullptr;
        usize m_reserved = 0;
        usize m_used = 0;
        usize m_committed = 0;
        bool m_is_initialized = false;
    };

} // namespace mantle
