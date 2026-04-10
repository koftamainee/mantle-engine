#pragma once

#include "core/types.h"

namespace mantle {

    class OSMemory final {
      public:
        OSMemory() = default;
        ~OSMemory() = default;

        OSMemory(const OSMemory &) = delete;
        OSMemory &operator=(const OSMemory &) = delete;
        OSMemory(OSMemory &&) = delete;
        OSMemory &operator=(OSMemory &&) = delete;

        void init();

        void *reserve(usize size) const;
        void commit(void *ptr, usize size) const;
        void decommit(void *ptr, usize size) const;
        void release(void *ptr, usize size) const;

        usize page_size() const;

      private:
        usize m_page_size = 0;
        bool m_is_initialized = false;
    };

} // namespace mantle
