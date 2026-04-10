#pragma once
#include "core/memory/os_memory.h"
#include "core/types.h"

namespace mantle {

    class VirtualHeap final {
      public:
        VirtualHeap() = default;
        ~VirtualHeap();

        VirtualHeap(const VirtualHeap &) = delete;
        VirtualHeap &operator=(const VirtualHeap &) = delete;
        VirtualHeap(VirtualHeap &&) = delete;
        VirtualHeap &operator=(VirtualHeap &&) = delete;

        void init(OSMemory &os, usize reserve_size);
        void destroy();

        [[nodiscard]] void *take(usize size);

        usize reserved() const;
        usize used() const;

      private:
        OSMemory *m_os = nullptr;
        void *m_base = nullptr;
        usize m_reserved = 0;
        usize m_used = 0;
        bool m_is_initialized = false;
    };

} // namespace mantle
