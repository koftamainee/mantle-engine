#pragma once

#include "core/macros.h"
#include "core/types.h"

namespace mantle {

    class OSMemory final {
      public:
        OSMemory() = default;
        ~OSMemory();

        MANTLE_NO_COPY_NO_MOVE(OSMemory);

        void init();
        void destroy();

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
