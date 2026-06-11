// Copyright (c) 2026 Mantle. All rights reserved.

#pragma once

#include "mantle/core/macros.h"
#include "mantle/core/memory/memory_block.h"
#include "mantle/core/memory/os_memory.h"
#include "mantle/core/types.h"

namespace mantle {

    class VirtualHeap final {
      public:
        VirtualHeap() = default;
        ~VirtualHeap();

        MANTLE_NO_COPY(VirtualHeap);

        void init(OSMemory &os, usize reserve_size);
        void destroy();

        [[nodiscard]] MemoryBlock take(usize size);

        usize reserved() const;
        usize used() const;
        usize committed() const;

      private:
        OSMemory *m_os = nullptr;
        void     *m_base = nullptr;
        usize     m_reserved = 0;
        usize     m_used = 0;
        usize     m_committed = 0;
        bool      m_is_initialized = false;
    };

} // namespace mantle
